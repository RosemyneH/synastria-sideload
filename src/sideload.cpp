#include "cache.h"
#include "features.h"
#include "sym.h"

#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

namespace {

// ʕ •ᴥ•ʔ✿ CMSG_UNUSED5. WoWExt.dll registers this with ClientServices::SetMessageHandler. ✿ ʕ •ᴥ•ʔ
constexpr int kOpcode = 0x4B8;
constexpr uint32_t kReplyTag = 0x36;
constexpr uint16_t kLoginVerifyWorld = 0x236;
constexpr int kLuaGlobals = -10002;
constexpr uint8_t kCharacterCap = 20;
constexpr char kLoginGuard[] =
    "if SynastriaLoginPatch then return end\n"
    "if not _G[\"CharacterSelectCharacterFrame\"] then return end\n"
    "SynastriaLoginPatch = true\n";

// ʕ •ᴥ•ʔ✿ Glue hides Create when count >= 10, then pins it under slot 14, off the frame. ✿ ʕ •ᴥ•ʔ
constexpr char kLoginFooter[] =
    "MAX_CHARACTERS_PER_REALM = 20\n"
    "MAX_CHARACTERS_DISPLAYED = 20\n"
    "local createButton = _G[\"CharSelectCreateCharacterButton\"]\n"
    "local createPoint, createRel, createRelPoint, createX, createY\n"
    "if createButton then\n"
    "  createPoint, createRel, createRelPoint, createX, createY = createButton:GetPoint(1)\n"
    "end\n"
    "local function SynastriaPlaceCreate()\n"
    "  if not createButton then\n"
    "    createButton = _G[\"CharSelectCreateCharacterButton\"]\n"
    "    if createButton and not createPoint then\n"
    "      createPoint, createRel, createRelPoint, createX, createY = createButton:GetPoint(1)\n"
    "    end\n"
    "  end\n"
    "  if not createButton then return end\n"
    "  local num = GetNumCharacters() or 0\n"
    "  if num >= 20 then\n"
    "    createButton:Hide()\n"
    "    return\n"
    "  end\n"
    "  if createPoint then\n"
    "    createButton:ClearAllPoints()\n"
    "    createButton:SetPoint(createPoint, createRel, createRelPoint, createX, createY)\n"
    "  end\n"
    "  createButton:Show()\n"
    "  if createButton.Enable then createButton:Enable() end\n"
    "end\n"
    "if type(UpdateCharacterList) == \"function\" then\n"
    "  local previous = UpdateCharacterList\n"
    "  function UpdateCharacterList(...)\n"
    "    local result = previous(...)\n"
    "    SynastriaPlaceCreate()\n"
    "    return result\n"
    "  end\n"
    "end\n"
    "SynastriaPlaceCreate()\n";

constexpr char kCreateButtonLua[] =
    "MAX_CHARACTERS_PER_REALM = 20\n"
    "MAX_CHARACTERS_DISPLAYED = 20\n"
    "local frame = _G[\"CharacterSelectCharacterFrame\"]\n"
    "local button = _G[\"CharSelectCreateCharacterButton\"]\n"
    "local function place()\n"
    "  if not button or not frame then return end\n"
    "  local num = GetNumCharacters() or 0\n"
    "  if num >= 20 then button:Hide() return end\n"
    "  button:ClearAllPoints()\n"
    "  button:SetPoint(\"BOTTOM\", frame, \"BOTTOM\", 0, 16)\n"
    "  button:SetFrameLevel((frame:GetFrameLevel() or 1) + 20)\n"
    "  button:Show()\n"
    "  if button.Enable then button:Enable() end\n"
    "end\n"
    "if frame and button and not SynastriaCreateHook and type(UpdateCharacterList) == \"function\" then\n"
    "  SynastriaCreateHook = true\n"
    "  local previous = UpdateCharacterList\n"
    "  function UpdateCharacterList(...)\n"
    "    MAX_CHARACTERS_PER_REALM = 20\n"
    "    MAX_CHARACTERS_DISPLAYED = 20\n"
    "    local result = previous(...)\n"
    "    place()\n"
    "    return result\n"
    "  end\n"
    "end\n"
    "place()\n";

constexpr uint8_t kPrologue[] = {
    0x41, 0x56,
    0x41, 0x89, 0xf6,
    0x55,
    0x48, 0x89, 0xfd,
    0x48, 0x89, 0xd7,
    0x53,
    0x48, 0x83, 0xec, 0x10,
};

using ProcessFn = int (*)(void*, uint32_t, void*, int);
using SetHandlerFn = void (*)(int, int (*)(void*, int, unsigned, void*), void*);
using SendFn = void (*)(void*);
using CtorFn = void (*)(void*);
using DtorFn = void (*)(void*);
using PutU8Fn = void (*)(void*, uint8_t);
using PutU32Fn = void (*)(void*, uint32_t);
using FinalizeFn = void (*)(void*);
using GetU8Fn = void (*)(void*, uint8_t*);
using GetContextFn = void* (*)();
using ExecuteFn = void (*)(const char*, const char*, const char*);
using LuaGetFieldFn = void (*)(void*, int, const char*);
using LuaToBooleanFn = int (*)(void*, int);
using LuaSetTopFn = void (*)(void*, int);

struct StoreView {
    void* vptr;
    uint8_t* data;
    uint32_t base;
    uint32_t alloc;
    uint32_t size;
    uint32_t read;
};

SetHandlerFn g_set_handler = nullptr;
SendFn g_send = nullptr;
CtorFn g_ctor = nullptr;
DtorFn g_dtor = nullptr;
PutU8Fn g_put_u8 = nullptr;
PutU32Fn g_put_u32 = nullptr;
FinalizeFn g_finalize = nullptr;
GetU8Fn g_get_u8 = nullptr;
GetContextFn g_lua_context = nullptr;
ExecuteFn g_lua_execute = nullptr;
LuaGetFieldFn g_lua_getfield = nullptr;
LuaToBooleanFn g_lua_toboolean = nullptr;
LuaSetTopFn g_lua_settop = nullptr;
char* g_login_lua = nullptr;
int g_lua_tries = 0;
void** g_connection = nullptr;
ProcessFn g_process = nullptr;

SynCache g_cache{};
FILE* g_log = nullptr;
uint32_t g_token = 0;
bool g_handler_ready = false;
bool g_announced = false;
bool g_create_ready = false;
bool g_lua_ready = false;
int g_depth = 0;

void log_line(const char* fmt, ...) {
    if (!g_log) {
        return;
    }
    va_list args;
    va_start(args, fmt);
    vfprintf(g_log, fmt, args);
    va_end(args);
    fputc('\n', g_log);
    fflush(g_log);
}

void log_stderr(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fputc('\n', stderr);
}

int hook_process(void* self, uint32_t time, void* msg, int unused);

void write_abs_jmp(uint8_t* at, void* dest) {
    at[0] = 0xFF;
    at[1] = 0x25;
    memset(at + 2, 0, 4);
    memcpy(at + 6, &dest, sizeof(dest));
}

bool patch_process(void* target) {
    auto* code = static_cast<uint8_t*>(target);
    if (memcmp(code, kPrologue, sizeof(kPrologue)) != 0) {
        char seen[80];
        size_t used = 0;
        for (size_t i = 0; i < sizeof(kPrologue) && used + 3 < sizeof(seen); ++i) {
            used += static_cast<size_t>(snprintf(seen + used, sizeof(seen) - used, "%02x", code[i]));
        }
        log_line("prologue mismatch at %p: %s", target, seen);
        log_stderr("synastria: prologue mismatch, sideload hook not installed");
        return false;
    }

    long page = sysconf(_SC_PAGESIZE);
    uintptr_t start = reinterpret_cast<uintptr_t>(code);
    uintptr_t page_start = start & ~(static_cast<uintptr_t>(page) - 1);
    size_t span = (start + sizeof(kPrologue)) - page_start;
    if (mprotect(reinterpret_cast<void*>(page_start), span, PROT_READ | PROT_WRITE | PROT_EXEC) != 0) {
        log_line("mprotect failed");
        return false;
    }

    uint8_t* tramp = static_cast<uint8_t*>(mmap(nullptr, static_cast<size_t>(page), PROT_READ | PROT_WRITE | PROT_EXEC,
                                                MAP_PRIVATE | MAP_ANONYMOUS, -1, 0));
    if (tramp == MAP_FAILED) {
        return false;
    }
    memcpy(tramp, code, sizeof(kPrologue));
    write_abs_jmp(tramp + sizeof(kPrologue), code + sizeof(kPrologue));
    g_process = reinterpret_cast<ProcessFn>(tramp);

    uint8_t patch[sizeof(kPrologue)];
    memset(patch, 0x90, sizeof(patch));
    write_abs_jmp(patch, reinterpret_cast<void*>(hook_process));
    memcpy(code, patch, sizeof(patch));
    mprotect(reinterpret_cast<void*>(page_start), span, PROT_READ | PROT_EXEC);
    return true;
}

bool can_send() {
    return g_connection && *g_connection && g_ctor && g_send && g_put_u8 && g_put_u32 && g_finalize && g_dtor;
}

void send_ack() {
    if (!can_send()) {
        return;
    }
    alignas(16) unsigned char raw[64];
    memset(raw, 0, sizeof(raw));
    g_ctor(raw);
    g_put_u32(raw, static_cast<uint32_t>(kOpcode));
    g_put_u8(raw, 1);
    g_put_u32(raw, kReplyTag);
    g_put_u32(raw, g_token);
    g_finalize(raw);
    g_send(raw);
    g_dtor(raw);
}

void send_presence() {
    if (g_announced || !can_send()) {
        return;
    }
    g_announced = true;
    send_ack();
    alignas(16) unsigned char raw[64];
    memset(raw, 0, sizeof(raw));
    g_ctor(raw);
    g_put_u32(raw, static_cast<uint32_t>(kOpcode));
    g_put_u8(raw, 0x1b);
    g_put_u8(raw, 1);
    g_put_u32(raw, 0);
    g_put_u8(raw, 0);
    g_finalize(raw);
    g_send(raw);
    g_dtor(raw);
    log_line("announced WoWExt on enter world");
    log_stderr("synastria: announced extension");
}

uint16_t peek_opcode(void* msg) {
    auto* store = static_cast<StoreView*>(msg);
    if (!store || !store->data || store->size < 2) {
        return 0xffff;
    }
    uint32_t pos = store->read > store->base ? store->read - store->base : 0;
    if (pos + 2 > store->size) {
        return 0xffff;
    }
    uint16_t id = 0;
    memcpy(&id, store->data + pos, 2);
    return id;
}

const uint8_t* payload(void* msg, uint32_t* nbytes) {
    auto* store = static_cast<StoreView*>(msg);
    *nbytes = 0;
    if (!store || !store->data || store->read < store->base || store->size < store->read) {
        return nullptr;
    }
    *nbytes = store->size - store->read;
    return store->data + (store->read - store->base);
}

int on_packet(void* /*param*/, int msg_id, unsigned /*time*/, void* msg) {
    uint8_t cmd = 0;
    if (g_get_u8 && msg) {
        g_get_u8(msg, &cmd);
    }

    uint32_t left = 0;
    const uint8_t* rest = payload(msg, &left);
    char hex[96];
    size_t used = 0;
    hex[0] = 0;
    uint32_t show = left < 24 ? left : 24;
    for (uint32_t i = 0; i < show && used + 3 < sizeof(hex); ++i) {
        used += static_cast<size_t>(snprintf(hex + used, sizeof(hex) - used, "%02x", rest[i]));
    }
    syn_features_packet(cmd, rest, left);
    log_line("pkt opcode=%#x cmd=%u token=%u rest=%u %s", msg_id, cmd, g_token, left, hex);

    if (cmd == 2 && rest && left >= 1 && g_token == 0 && rest[0] != 0) {
        g_token = rest[0];
        log_line("token set to %u", g_token);
    }
    if (cmd == 1) {
        send_ack();
        log_line("ack opcode=%#x tag=%#x token=%u", kOpcode, kReplyTag, g_token);
    }
    return 1;
}

void ensure_handler() {
    if (g_handler_ready || !g_set_handler) {
        return;
    }
    if (!g_connection || !*g_connection) {
        return;
    }
    g_set_handler(kOpcode, on_packet, nullptr);
    g_handler_ready = true;
    log_line("registered handler for opcode %#x", kOpcode);
}

bool patch_character_cap() {
    void* target = nullptr;
    if (!syn_resolve("_ZN15RealmConnection14HandleCharEnumEjjP10CDataStore", &target)) {
        log_line("missing HandleCharEnum");
        return false;
    }

    auto* code = static_cast<uint8_t*>(target);
    uint8_t* slot = nullptr;
    for (size_t i = 0; i + 4 < 0x80; ++i) {
        if (code[i] == 0x3c && code[i + 1] == 0x0a && code[i + 2] == 0x0f && code[i + 3] == 0x86) {
            slot = code + i + 1;
            break;
        }
    }
    if (!slot) {
        log_line("character cap compare not found at %p", target);
        log_stderr("synastria: character cap patch missed");
        return false;
    }

    long page = sysconf(_SC_PAGESIZE);
    uintptr_t start = reinterpret_cast<uintptr_t>(slot);
    uintptr_t page_start = start & ~(static_cast<uintptr_t>(page) - 1);
    if (mprotect(reinterpret_cast<void*>(page_start), static_cast<size_t>(page), PROT_READ | PROT_WRITE | PROT_EXEC) != 0) {
        log_line("mprotect character cap failed");
        return false;
    }
    *slot = kCharacterCap;
    mprotect(reinterpret_cast<void*>(page_start), static_cast<size_t>(page), PROT_READ | PROT_EXEC);
    log_line("character list cap %u at %p", kCharacterCap, slot);
    log_stderr("synastria: character list cap %u", kCharacterCap);
    return true;
}

void prepare_login_lua() {
    if (!g_cache.lua) {
        return;
    }
    size_t guard = sizeof(kLoginGuard) - 1;
    size_t footer = sizeof(kLoginFooter) - 1;
    g_login_lua = static_cast<char*>(malloc(guard + g_cache.lua_bytes + footer + 1));
    if (!g_login_lua) {
        return;
    }
    memcpy(g_login_lua, kLoginGuard, guard);
    memcpy(g_login_lua + guard, g_cache.lua, g_cache.lua_bytes);
    memcpy(g_login_lua + guard + g_cache.lua_bytes, kLoginFooter, footer);
    g_login_lua[guard + g_cache.lua_bytes + footer] = 0;
}

void ensure_lua() {
    if (!g_lua_context || !g_lua_execute) {
        return;
    }
    void* state = g_lua_context();
    if (!state) {
        return;
    }

    if (!g_create_ready) {
        g_lua_execute(kCreateButtonLua, "SynastriaCreate.lua", "synastria");
        if (g_lua_getfield && g_lua_toboolean && g_lua_settop) {
            g_lua_getfield(state, kLuaGlobals, "SynastriaCreateHook");
            g_create_ready = g_lua_toboolean(state, -1) != 0;
            g_lua_settop(state, -2);
        }
        if (g_create_ready) {
            log_line("create character button kept for accounts under 20");
        }
    }

    if (g_lua_ready || !g_login_lua) {
        return;
    }
    if (g_lua_tries++ > 200) {
        g_lua_ready = true;
        log_line("login patch gave up waiting for CharacterSelect");
        return;
    }

    g_lua_execute(g_login_lua, "WoWExt3.bin", "synastria");
    if (!g_lua_getfield || !g_lua_toboolean || !g_lua_settop) {
        g_lua_ready = true;
        log_line("executed WoWExt3.bin (%zu bytes)", g_cache.lua_bytes);
        return;
    }

    g_lua_getfield(state, kLuaGlobals, "SynastriaLoginPatch");
    int applied = g_lua_toboolean(state, -1);
    g_lua_settop(state, -2);
    if (!applied) {
        return;
    }
    g_lua_ready = true;
    log_line("login patch applied from WoWExt3.bin (%zu bytes)", g_cache.lua_bytes);
    log_stderr("synastria: login patch applied");
}

int hook_process(void* self, uint32_t time, void* msg, int unused) {
    if (g_depth++ == 0) {
        ensure_handler();
        syn_features_before();
        if (peek_opcode(msg) == kLoginVerifyWorld) {
            send_presence();
        }
        ensure_lua();
    }
    int result = g_process(self, time, msg, unused);
    if (--g_depth == 0) {
        syn_features_after();
    }
    return result;
}

bool bind_symbol(const char* name, void** slot) {
    void* addr = nullptr;
    if (!syn_resolve(name, &addr)) {
        log_line("missing symbol %s", name);
        log_stderr("synastria: missing %s", name);
        return false;
    }
    *slot = addr;
    return true;
}

bool resolve_all() {
    return bind_symbol("_ZN14ClientServices17SetMessageHandlerE10NETMESSAGEPFiPvS0_jP10CDataStoreES1_",
                        reinterpret_cast<void**>(&g_set_handler))
        && bind_symbol("_ZN14ClientServices5Send2EP10CDataStore", reinterpret_cast<void**>(&g_send))
        && bind_symbol("_ZN14ClientServices19s_currentConnectionE", reinterpret_cast<void**>(&g_connection))
        && bind_symbol("_ZN10CDataStoreC1Ev", reinterpret_cast<void**>(&g_ctor))
        && bind_symbol("_ZN10CDataStoreD1Ev", reinterpret_cast<void**>(&g_dtor))
        && bind_symbol("_ZN10CDataStore3PutEh", reinterpret_cast<void**>(&g_put_u8))
        && bind_symbol("_ZN10CDataStore3PutEj", reinterpret_cast<void**>(&g_put_u32))
        && bind_symbol("_ZN10CDataStore8FinalizeEv", reinterpret_cast<void**>(&g_finalize))
        && bind_symbol("_ZN10CDataStore3GetERh", reinterpret_cast<void**>(&g_get_u8))
        && bind_symbol("_Z22FrameScript_GetContextv", reinterpret_cast<void**>(&g_lua_context))
        && bind_symbol("_Z19FrameScript_ExecutePKcS0_S0_", reinterpret_cast<void**>(&g_lua_execute))
        && bind_symbol("lua_getfield", reinterpret_cast<void**>(&g_lua_getfield))
        && bind_symbol("lua_toboolean", reinterpret_cast<void**>(&g_lua_toboolean))
        && bind_symbol("lua_settop", reinterpret_cast<void**>(&g_lua_settop));
}

const char* root_dir() {
    const char* env = getenv("SYNASTRIA_ROOT");
    if (env && env[0]) {
        return env;
    }
    return "/home/emma/Games/Synastria";
}

// ʕ •ᴥ•ʔ✿ Children such as zenity must not inherit the bridge. The client already has it loaded. ✿ ʕ •ᴥ•ʔ
void drop_preload() {
    const char* preload = getenv("LD_PRELOAD");
    if (!preload || !strstr(preload, "libsynastria.so")) {
        return;
    }
    char kept[4096];
    kept[0] = 0;
    char copy[4096];
    snprintf(copy, sizeof(copy), "%s", preload);
    char* save = nullptr;
    for (char* part = strtok_r(copy, " :", &save); part; part = strtok_r(nullptr, " :", &save)) {
        if (strstr(part, "libsynastria.so")) {
            continue;
        }
        if (kept[0]) {
            strncat(kept, ":", sizeof(kept) - strlen(kept) - 1);
        }
        strncat(kept, part, sizeof(kept) - strlen(kept) - 1);
    }
    if (kept[0]) {
        setenv("LD_PRELOAD", kept, 1);
    } else {
        unsetenv("LD_PRELOAD");
    }
}

void describe_cache() {
    log_line("WoWExt.bin %s %zu", g_cache.bin_ok ? "ok" : "bad", g_cache.bin_bytes);
    log_line("WoWExt2.bin %s version %u %zu", g_cache.bin2_ok ? "ok" : "bad", g_cache.bin2_version, g_cache.bin2_bytes);
    log_line("WoWExt3.bin %s %zu", g_cache.lua_ok ? "ok" : "bad", g_cache.lua_bytes);
    log_line("WoWExtItemLoc.bin %s version %u %zu", g_cache.item_ok ? "ok" : "bad", g_cache.item_version, g_cache.item_bytes);
    log_line("WoWExtObjLoc.bin %s version %u count %u %zu",
             g_cache.obj_ok ? "ok" : "bad", g_cache.obj_version, g_cache.obj_count, g_cache.obj_bytes);
    log_line("WoWExt.dll %s %zu (kept on disk, not mapped)", g_cache.dll_present ? "pe32" : "missing", g_cache.dll_bytes);
}

void install() {
    drop_preload();

    char exe[4096];
    ssize_t n = readlink("/proc/self/exe", exe, sizeof(exe) - 1);
    if (n <= 0) {
        return;
    }
    exe[n] = 0;
    const char* slash = strrchr(exe, '/');
    const char* base = slash ? slash + 1 : exe;
    if (strncmp(base, "WowClient", 9) != 0) {
        return;
    }

    const char* root = root_dir();
    char logs[1024];
    snprintf(logs, sizeof(logs), "%s/Logs", root);
    mkdir(logs, 0755);

    snprintf(logs, sizeof(logs), "%s/Logs/synastria-sideload.log", root);
    g_log = fopen(logs, "w");
    syn_features_set_log(g_log);

    syn_load_cache(root, &g_cache);
    syn_features_load(root);
    prepare_login_lua();
    describe_cache();
    log_line("pid %d", getpid());
    log_stderr("synastria: cache %s pid %d", (g_cache.bin_ok && g_cache.bin2_ok && g_cache.lua_ok) ? "ok" : "incomplete", getpid());

    if (!resolve_all()) {
        return;
    }
    void* process = nullptr;
    if (!syn_resolve("_ZN9NetClient14ProcessMessageEjP10CDataStorei", &process)) {
        log_line("missing ProcessMessage");
        return;
    }
    if (patch_process(process)) {
        log_line("hooked NetClient::ProcessMessage at %p", process);
        log_stderr("synastria: hooked opcode %#x", kOpcode);
    }
    patch_character_cap();
}

}  // namespace

__attribute__((constructor))
static void syn_construct() {
    install();
}

__attribute__((destructor))
static void syn_destruct() {
    if (g_log) {
        fclose(g_log);
        g_log = nullptr;
    }
    free(g_login_lua);
    g_login_lua = nullptr;
    syn_free_cache(&g_cache);
}
