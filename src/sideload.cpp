#include "cache.h"
#include "features.h"
#include "sym.h"

#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <execinfo.h>
#include <signal.h>
#include <cstdlib>
#include <cstring>
#include <string>
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
    "if SynastriaLoginPatch or SynastriaLoginStarted then return end\n"
    "if type(UpdateCharacterList) ~= \"function\" or type(CharacterClass_OnClick) ~= \"function\" then return end\n"
    "if not _G[\"CharacterSelectCharacterFrame\"] or not _G[\"CharSelectCharacterButton10\"] then return end\n"
    "SynastriaLoginStarted = true\n"
    "local ok, err = pcall(function()\n";

constexpr char kLoginTail[] =
    "\nend)\n"
    "if _G[\"CharSelectCharacterButton11\"] and orig_CharacterClass_OnClick then\n"
    "  SynastriaLoginPatch = true\n"
    "elseif not ok then\n"
    "  SynastriaLoginError = tostring(err)\n"
    "end\n";

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
    "if not SynastriaInfoHook and type(GetCharacterInfo) == \"function\" then\n"
    "  SynastriaInfoHook = true\n"
    "  local rawInfo = GetCharacterInfo\n"
    "  function GetCharacterInfo(index, ...)\n"
    "    if SynastriaHarvest then SynastriaHarvest() end\n"
    "    local name, race, class, level, zone, sex, ghost, pcc, prc, pfc = rawInfo(index, ...)\n"
    "    local combo = SynastriaCharacterClass and SynastriaCharacterClass(index)\n"
    "    if combo then class = combo end\n"
    "    return name, race, class, level, zone, sex, ghost, pcc, prc, pfc\n"
    "  end\n"
    "end\n"
    "SynastriaPlaceCreate()\n";

constexpr char kCreateButtonLua[] =
    "MAX_CHARACTERS_PER_REALM = 20\n"
    "MAX_CHARACTERS_DISPLAYED = 20\n"
    "local function place()\n"
    "  local frame = _G[\"CharacterSelectCharacterFrame\"]\n"
    "  local button = _G[\"CharSelectCreateCharacterButton\"]\n"
    "  if not button or not frame then return end\n"
    "  local num = GetNumCharacters() or 0\n"
    "  if num >= 20 then button:Hide() return end\n"
    "  button:ClearAllPoints()\n"
    "  button:SetPoint(\"BOTTOM\", frame, \"BOTTOM\", 0, 16)\n"
    "  button:SetFrameLevel((frame:GetFrameLevel() or 1) + 20)\n"
    "  button:Show()\n"
    "  if button.Enable then button:Enable() end\n"
    "end\n"
    "pcall(function()\n"
    "  local parent = _G[\"CharacterSelectCharacterFrame\"]\n"
    "  if parent then\n"
    "    for i = 11, 20 do\n"
    "      if not _G[\"CharSelectCharacterButton\" .. i] then\n"
    "        local prev = _G[\"CharSelectCharacterButton\" .. (i - 1)]\n"
    "        if prev then\n"
    "          local btn = CreateFrame(\"Button\", \"CharSelectCharacterButton\" .. i, parent, \"CharSelectCharacterButtonTemplate\", i)\n"
    "          btn:SetPoint(\"TOP\", prev, \"BOTTOM\", 0, 8)\n"
    "          btn:SetSize(256, 35)\n"
    "        end\n"
    "      end\n"
    "    end\n"
    "  end\n"
    "end)\n"
    "if not SynastriaCreateHook and type(UpdateCharacterList) == \"function\" then\n"
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
using HandleDataFn = int (*)(void*, uint32_t, void*, int);
using IdleFn = int (*)(const void*, void*);
using SetHandlerFn = void (*)(int, int (*)(void*, int, unsigned, void*), void*);
using NetSetHandlerFn = void (*)(void*, int, int (*)(void*, int, unsigned, void*), void*);
using NetSendFn = void (*)(void*, void*);
using SendFn = void (*)(void*);
using CtorFn = void (*)(void*);
using DtorFn = void (*)(void*);
using PutU8Fn = void (*)(void*, uint8_t);
using PutU32Fn = void (*)(void*, uint32_t);
using PutStrFn = void (*)(void*, const char*);
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
NetSetHandlerFn g_net_set = nullptr;
NetSendFn g_net_send = nullptr;
void* g_net = nullptr;
SendFn g_send = nullptr;
CtorFn g_ctor = nullptr;
DtorFn g_dtor = nullptr;
PutU8Fn g_put_u8 = nullptr;
PutU32Fn g_put_u32 = nullptr;
PutStrFn g_put_str = nullptr;
FinalizeFn g_finalize = nullptr;
GetU8Fn g_get_u8 = nullptr;
GetContextFn g_lua_context = nullptr;
ExecuteFn g_lua_execute = nullptr;
LuaGetFieldFn g_lua_getfield = nullptr;
LuaToBooleanFn g_lua_toboolean = nullptr;
LuaSetTopFn g_lua_settop = nullptr;
using LuaToStringFn = const char* (*)(void*, int, size_t*);
LuaToStringFn g_lua_tolstring = nullptr;
char* g_login_lua = nullptr;
IdleFn g_glue_idle = nullptr;
IdleFn g_game_idle = nullptr;
void** g_connection = nullptr;
ProcessFn g_process = nullptr;
HandleDataFn g_handle_data = nullptr;

SynCache g_cache{};
FILE* g_log = nullptr;
uint32_t g_token = 0;
bool g_handler_ready = false;
bool g_announced = false;
bool g_create_ready = false;
bool g_lua_ready = false;
bool g_login_error_logged = false;
void* g_seen_lua = nullptr;
int g_depth = 0;
int g_main_depth = 0;

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

void* detour(void* target, size_t steal, void* hook, const char* label) {
    auto* code = static_cast<uint8_t*>(target);
    long page = sysconf(_SC_PAGESIZE);
    uintptr_t start = reinterpret_cast<uintptr_t>(code);
    uintptr_t page_start = start & ~(static_cast<uintptr_t>(page) - 1);
    size_t span = static_cast<size_t>(page) * 2;
    if (mprotect(reinterpret_cast<void*>(page_start), span, PROT_READ | PROT_WRITE | PROT_EXEC) != 0) {
        log_line("mprotect failed for %s", label);
        return nullptr;
    }

    uint8_t* tramp = static_cast<uint8_t*>(mmap(nullptr, static_cast<size_t>(page), PROT_READ | PROT_WRITE | PROT_EXEC,
                                                MAP_PRIVATE | MAP_ANONYMOUS, -1, 0));
    if (tramp == MAP_FAILED) {
        return nullptr;
    }
    memcpy(tramp, code, steal);
    write_abs_jmp(tramp + steal, code + steal);

    uint8_t patch[32];
    memset(patch, 0x90, steal);
    write_abs_jmp(patch, hook);
    memcpy(code, patch, steal);
    mprotect(reinterpret_cast<void*>(page_start), span, PROT_READ | PROT_EXEC);
    log_line("hooked %s at %p", label, target);
    return tramp;
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
    void* tramp = detour(target, sizeof(kPrologue), reinterpret_cast<void*>(hook_process), "NetClient::ProcessMessage");
    if (!tramp) {
        return false;
    }
    g_process = reinterpret_cast<ProcessFn>(tramp);
    return true;
}

bool can_send() {
    return g_ctor && g_put_u8 && g_put_u32 && g_finalize && g_dtor && (g_net_send || g_send);
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
    if (g_net_send && g_net) {
        g_net_send(g_net, raw);
    } else if (g_send) {
        g_send(raw);
    }
    g_dtor(raw);
}

void syn_net_notify_impl(uint32_t a, uint32_t b, const char* text) {
    if (!can_send() || !g_put_str) {
        return;
    }
    alignas(16) unsigned char raw[64];
    memset(raw, 0, sizeof(raw));
    g_ctor(raw);
    g_put_u32(raw, static_cast<uint32_t>(kOpcode));
    g_put_u8(raw, 0x0c);
    g_put_u32(raw, a);
    g_put_u32(raw, b);
    if (text) {
        g_put_str(raw, text);
    } else {
        g_put_u8(raw, 0);
    }
    g_finalize(raw);
    if (g_net_send && g_net) {
        g_net_send(g_net, raw);
    } else if (g_send) {
        g_send(raw);
    }
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
    if (g_net_send && g_net) {
        g_net_send(g_net, raw);
    } else if (g_send) {
        g_send(raw);
    }
    g_dtor(raw);
    log_line("announced WoWExt on enter world");
    log_stderr("synastria: announced extension");
}

void send_cache_reply(const uint32_t* ids, int count) {
    if (!can_send() || count < 0) {
        return;
    }
    alignas(16) unsigned char raw[64];
    memset(raw, 0, sizeof(raw));
    g_ctor(raw);
    g_put_u32(raw, static_cast<uint32_t>(kOpcode));
    g_put_u8(raw, 3);
    g_put_u32(raw, static_cast<uint32_t>(count));
    for (int i = 0; i < count; ++i) {
        g_put_u32(raw, ids[i]);
    }
    g_finalize(raw);
    if (g_net_send && g_net) {
        g_net_send(g_net, raw);
    } else if (g_send) {
        g_send(raw);
    }
    g_dtor(raw);
    log_line("container sync reply count=%d", count);
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

    if (cmd == 2 && rest && left >= 1 && rest[0] == 0) {
        uint32_t missing[256];
        int count = syn_features_cache_missing(rest, left, missing, 256);
        if (count >= 0) {
            send_cache_reply(missing, count);
        }
    } else if (cmd == 2 && rest && left >= 1 && g_token == 0 && rest[0] != 0) {
        g_token = rest[0];
        log_line("token set to %u", g_token);
    }
    if (cmd == 1) {
        send_ack();
        log_line("ack opcode=%#x tag=%#x token=%u", kOpcode, kReplyTag, g_token);
    }
    return 1;
}

void ensure_handler(void* net) {
    if (net) {
        g_net = net;
    }
    if (g_handler_ready || !g_net_set || !g_net) {
        return;
    }
    g_net_set(g_net, kOpcode, on_packet, nullptr);
    g_handler_ready = true;
    log_line("registered handler for opcode %#x on %p", kOpcode, g_net);
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
    std::string body(g_cache.lua, g_cache.lua + g_cache.lua_bytes);
    const char* slots[] = {
        "CharSelectCharacterButton",
        "CharSelectCharacterCustomize",
        "CharSelectRaceChange",
        "CharSelectFactionChange",
    };
    for (const char* name : slots) {
        std::string from = std::string("CreateFrame(\"Button\", \"") + name + "\" .. i";
        std::string to = std::string("_G[\"") + name + R"(" .. i] or CreateFrame("Button", ")" + name + R"(" .. i)";
        size_t pos = body.find(from);
        if (pos == std::string::npos) {
            log_line("login lua missing create for %s", name);
            continue;
        }
        body.replace(pos, from.size(), to);
    }
    std::string script = std::string(kLoginGuard) + body + kLoginFooter + kLoginTail;
    g_login_lua = strdup(script.c_str());
}

void ensure_lua() {
    if (!g_lua_context || !g_lua_execute) {
        return;
    }
    void* state = g_lua_context();
    if (!state) {
        return;
    }

    if (!g_lua_ready && g_login_lua) {
        g_lua_execute(g_login_lua, "WoWExt3.bin", "synastria");
        int applied = 0;
        if (g_lua_getfield && g_lua_toboolean && g_lua_settop) {
            g_lua_getfield(state, kLuaGlobals, "SynastriaLoginPatch");
            applied = g_lua_toboolean(state, -1);
            g_lua_settop(state, -2);
            if (!applied && g_lua_tolstring && !g_login_error_logged) {
                g_lua_getfield(state, kLuaGlobals, "SynastriaLoginError");
                const char* err = g_lua_tolstring(state, -1, nullptr);
                if (err && err[0]) {
                    log_line("login patch error: %s", err);
                    g_login_error_logged = true;
                    g_lua_ready = true;
                }
                g_lua_settop(state, -2);
            }
        }
        int started = 0;
        if (!applied && g_lua_getfield && g_lua_toboolean && g_lua_settop) {
            g_lua_getfield(state, kLuaGlobals, "SynastriaLoginStarted");
            started = g_lua_toboolean(state, -1);
            g_lua_settop(state, -2);
        }
        if (applied) {
            g_lua_ready = true;
            log_line("login patch applied from WoWExt3.bin (%zu bytes)", g_cache.lua_bytes);
            log_stderr("synastria: login patch applied");
        } else if (started) {
            g_lua_ready = true;
            log_line("login patch stopped after one attempt");
        }
    }

    if (!g_lua_ready || g_create_ready) {
        return;
    }
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

int hook_process(void* self, uint32_t time, void* msg, int unused) {
    static int entries = 0;
    if (entries++ == 0) {
        log_line("hook entered");
    }
    if (g_depth++ == 0) {
        ensure_handler(self);
        if (peek_opcode(msg) == kLoginVerifyWorld) {
            syn_features_world();
            send_presence();
        }
    }
    int result = g_process(self, time, msg, unused);
    --g_depth;
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
        && bind_symbol("_ZN9NetClient17SetMessageHandlerE10NETMESSAGEPFiPvS0_jP10CDataStoreES1_",
                        reinterpret_cast<void**>(&g_net_set))
        && bind_symbol("_ZN9NetClient4SendEP10CDataStore", reinterpret_cast<void**>(&g_net_send))
        && bind_symbol("_ZN14ClientServices5Send2EP10CDataStore", reinterpret_cast<void**>(&g_send))
        && bind_symbol("_ZN14ClientServices19s_currentConnectionE", reinterpret_cast<void**>(&g_connection))
        && bind_symbol("_ZN10CDataStoreC1Ev", reinterpret_cast<void**>(&g_ctor))
        && bind_symbol("_ZN10CDataStoreD1Ev", reinterpret_cast<void**>(&g_dtor))
        && bind_symbol("_ZN10CDataStore3PutEh", reinterpret_cast<void**>(&g_put_u8))
        && bind_symbol("_ZN10CDataStore3PutEj", reinterpret_cast<void**>(&g_put_u32))
        && bind_symbol("_ZN10CDataStore9PutStringEPKc", reinterpret_cast<void**>(&g_put_str))
        && bind_symbol("_ZN10CDataStore8FinalizeEv", reinterpret_cast<void**>(&g_finalize))
        && bind_symbol("_ZN10CDataStore3GetERh", reinterpret_cast<void**>(&g_get_u8))
        && bind_symbol("_Z22FrameScript_GetContextv", reinterpret_cast<void**>(&g_lua_context))
        && bind_symbol("_Z19FrameScript_ExecutePKcS0_S0_", reinterpret_cast<void**>(&g_lua_execute))
        && bind_symbol("lua_getfield", reinterpret_cast<void**>(&g_lua_getfield))
        && bind_symbol("lua_toboolean", reinterpret_cast<void**>(&g_lua_toboolean))
        && bind_symbol("lua_settop", reinterpret_cast<void**>(&g_lua_settop))
        && bind_symbol("lua_tolstring", reinterpret_cast<void**>(&g_lua_tolstring));
}

constexpr uint8_t kHandleData[] = {
    0x41, 0x57,
    0x41, 0x89, 0xf7,
    0x41, 0x56,
    0x53,
    0x48, 0x89, 0xfb,
    0x48, 0x83, 0xec, 0x50,
};

int hook_handle_data(void* self, uint32_t time, void* data, int size) {
    static int entries = 0;
    if (entries++ == 0) {
        log_line("handle data entered");
    }
    ensure_handler(self);
    if (data && size >= 2) {
        uint16_t opcode = 0;
        memcpy(&opcode, data, sizeof(opcode));
        if (opcode == kLoginVerifyWorld) {
            syn_features_world();
            send_presence();
        }
    }
    return g_handle_data(self, time, data, size);
}

void allow_extension_code() {
    void* target = nullptr;
    if (!syn_resolve("_Z25OsValidateFunctionPointerPKv", &target)) {
        log_line("missing OsValidateFunctionPointer");
        return;
    }
    auto* code = static_cast<uint8_t*>(target);
    long page = sysconf(_SC_PAGESIZE);
    uintptr_t start = reinterpret_cast<uintptr_t>(code);
    uintptr_t page_start = start & ~(static_cast<uintptr_t>(page) - 1);
    if (mprotect(reinterpret_cast<void*>(page_start), static_cast<size_t>(page), PROT_READ | PROT_WRITE | PROT_EXEC) != 0) {
        log_line("mprotect failed for function pointer check");
        return;
    }
    code[0] = 0xc3;
    mprotect(reinterpret_cast<void*>(page_start), static_cast<size_t>(page), PROT_READ | PROT_EXEC);
    log_line("allowed extension lua callbacks");
}

constexpr uint8_t kGlueIdle[] = {
    0x48, 0x81, 0xec, 0xc8, 0x03, 0x00, 0x00,
    0x48, 0x89, 0x9c, 0x24, 0xa0, 0x03, 0x00, 0x00,
};
constexpr uint8_t kGameIdle[] = {
    0x48, 0x81, 0xec, 0xf8, 0x0b, 0x00, 0x00,
    0x4c, 0x89, 0xb4, 0x24, 0xe8, 0x0b, 0x00, 0x00,
};

struct sigaction g_prev_segv{};

void syn_segv(int sig, siginfo_t* info, void* ctx) {
    dprintf(2, "synastria: fault addr=%p code=%d\n", info ? info->si_addr : nullptr, info ? info->si_code : 0);
    void* frames[48];
    int count = backtrace(frames, 48);
    backtrace_symbols_fd(frames, count, 2);
    if (g_prev_segv.sa_flags & SA_SIGINFO) {
        if (g_prev_segv.sa_sigaction) {
            g_prev_segv.sa_sigaction(sig, info, ctx);
        }
        return;
    }
    if (g_prev_segv.sa_handler && g_prev_segv.sa_handler != SIG_DFL && g_prev_segv.sa_handler != SIG_IGN) {
        g_prev_segv.sa_handler(sig);
        return;
    }
    signal(sig, SIG_DFL);
    raise(sig);
}

void install_fault_trace() {
    static bool armed = false;
    if (armed) {
        return;
    }
    struct sigaction next{};
    next.sa_sigaction = syn_segv;
    sigemptyset(&next.sa_mask);
    next.sa_flags = SA_SIGINFO;
    if (sigaction(SIGSEGV, &next, &g_prev_segv) == 0) {
        armed = true;
    }
}

void pump_main(bool in_game) {
    install_fault_trace();
    static int seen_glue = 0;
    static int seen_game = 0;
    if (in_game) {
        if (seen_game++ == 0) {
            log_line("game idle entered");
        }
    } else if (seen_glue++ == 0) {
        log_line("glue idle entered");
    }
    if (g_main_depth) {
        return;
    }
    g_main_depth++;
    if (g_lua_context) {
        void* state = g_lua_context();
        if (state && state != g_seen_lua) {
            if (g_seen_lua) {
                log_line("lua state reset, reinstalling synastria ui");
                g_lua_ready = false;
                g_create_ready = false;
                g_login_error_logged = false;
                syn_features_reset();
            }
            g_seen_lua = state;
        }
    }
    syn_features_before();
    syn_features_harvest();
    ensure_lua();
    if (in_game) {
        syn_features_after();
    }
    g_main_depth--;
}

int hook_glue_idle(const void* data, void* param) {
    pump_main(false);
    return g_glue_idle(data, param);
}

int hook_game_idle(const void* data, void* param) {
    pump_main(true);
    return g_game_idle(data, param);
}

void hook_idle(const char* symbol, const uint8_t* prefix, size_t bytes, void* hook, IdleFn* slot, const char* label) {
    void* target = nullptr;
    if (!syn_resolve(symbol, &target)) {
        log_line("missing %s", label);
        return;
    }
    if (memcmp(target, prefix, bytes) != 0) {
        log_line("%s prologue mismatch", label);
        return;
    }
    void* tramp = detour(target, bytes, hook, label);
    if (tramp) {
        *slot = reinterpret_cast<IdleFn>(tramp);
    }
}

constexpr uint8_t kItemLookup[] = {
    0x48, 0x83, 0xec, 0x28,
    0x48, 0x89, 0x6c, 0x24, 0x18,
    0x64, 0x48, 0x8b, 0x2c, 0x25, 0x28, 0x00, 0x00, 0x00,
    0x48, 0x89, 0x6c, 0x24, 0x08,
    0x89, 0xfd,
};

constexpr uint8_t kFullItem[] = {
    0x48, 0x81, 0xec, 0x88, 0x00, 0x00, 0x00,
    0x48, 0x89, 0x5c, 0x24, 0x58,
    0x48, 0x89, 0xcb,
};

struct ItemSlot {
    void* (*original)(int);
};

ItemSlot g_item_slots[24]{};

extern "C" void* syn_item_hook(ItemSlot* slot, int id) {
    // ʕ •ᴥ•ʔ✿ A miss must stay empty so the server item query can fill real stat values. ✿ ʕ •ᴥ•ʔ
    return slot && slot->original ? slot->original(id) : nullptr;
}

uint8_t* emit_item_stub(uint8_t* at, ItemSlot* slot) {
    auto imm = [&](uint64_t value) {
        memcpy(at, &value, 8);
        at += 8;
    };
    *at++ = 0x57;
    *at++ = 0x48;
    *at++ = 0xbf;
    imm(reinterpret_cast<uint64_t>(slot));
    *at++ = 0x8b;
    *at++ = 0x34;
    *at++ = 0x24;
    *at++ = 0x48;
    *at++ = 0xb8;
    imm(reinterpret_cast<uint64_t>(&syn_item_hook));
    *at++ = 0xff;
    *at++ = 0xd0;
    *at++ = 0x48;
    *at++ = 0x83;
    *at++ = 0xc4;
    *at++ = 0x08;
    *at++ = 0xc3;
    return at;
}

using FullItemFn = void* (*)(void*, int, void*, void*, void*, bool);
FullItemFn g_full_item = nullptr;

void* hook_full_item(void* self, int id, void* guid, void* cb, void* user, bool create) {
    return g_full_item ? g_full_item(self, id, guid, cb, user, create) : nullptr;
}

void hook_item_records() {
    void* copies[24];
    int count = syn_resolve_each(
        "_ZN7DBCacheI11ItemStats_Ci11HASHKEY_INTE9GetRecordEiRK7WOWGUIDPFviS5_PvbES6_b.constprop.0",
        copies, 24);
    long page = sysconf(_SC_PAGESIZE);
    uint8_t* stub = static_cast<uint8_t*>(mmap(nullptr, static_cast<size_t>(page), PROT_READ | PROT_WRITE | PROT_EXEC,
                                               MAP_PRIVATE | MAP_ANONYMOUS, -1, 0));
    if (stub == MAP_FAILED) {
        log_line("item lookup stub alloc failed");
        return;
    }
    int hooked = 0;
    for (int i = 0; i < count && hooked < 24; ++i) {
        auto* code = static_cast<uint8_t*>(copies[i]);
        if (memcmp(code, kItemLookup, sizeof(kItemLookup)) != 0) {
            continue;
        }
        uint8_t* next = emit_item_stub(stub, &g_item_slots[hooked]);
        void* tramp = detour(copies[i], 18, stub, "item lookup");
        if (!tramp) {
            continue;
        }
        g_item_slots[hooked].original = reinterpret_cast<void* (*)(int)>(tramp);
        stub = next;
        ++hooked;
    }
    void* full = nullptr;
    if (syn_resolve("_ZN7DBCacheI11ItemStats_Ci11HASHKEY_INTE9GetRecordEiRK7WOWGUIDPFviS5_PvbES6_b", &full)
        && memcmp(full, kFullItem, sizeof(kFullItem)) == 0) {
        void* tramp = detour(full, sizeof(kFullItem), reinterpret_cast<void*>(hook_full_item), "item lookup full");
        if (tramp) {
            g_full_item = reinterpret_cast<FullItemFn>(tramp);
            ++hooked;
        }
    }
    log_line("item lookup hooks %d", hooked);
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
    syn_features_prepare(root);
    prepare_login_lua();
    describe_cache();
    log_line("pid %d", getpid());
    log_stderr("synastria: cache %s pid %d", (g_cache.bin_ok && g_cache.bin2_ok && g_cache.lua_ok) ? "ok" : "incomplete", getpid());

    if (!resolve_all()) {
        return;
    }
    hook_item_records();
    void* process = nullptr;
    if (!syn_resolve("_ZN9NetClient14ProcessMessageEjP10CDataStorei", &process)) {
        log_line("missing ProcessMessage");
        return;
    }
    if (patch_process(process)) {
        log_stderr("synastria: hooked opcode %#x", kOpcode);
    }
    void* handle_data = nullptr;
    if (syn_resolve("_ZN9NetClient10HandleDataEjPvi", &handle_data)
        && memcmp(handle_data, kHandleData, sizeof(kHandleData)) == 0) {
        void* tramp = detour(handle_data, sizeof(kHandleData), reinterpret_cast<void*>(hook_handle_data), "NetClient::HandleData");
        if (tramp) {
            g_handle_data = reinterpret_cast<HandleDataFn>(tramp);
        }
    } else {
        log_line("HandleData prologue mismatch");
    }
    allow_extension_code();
    hook_idle("_ZN8CGlueMgr4IdleEPKvPv", kGlueIdle, sizeof(kGlueIdle),
              reinterpret_cast<void*>(hook_glue_idle), &g_glue_idle, "CGlueMgr::Idle");
    hook_idle("_ZN8CGGameUI4IdleEPKvPv", kGameIdle, sizeof(kGameIdle),
              reinterpret_cast<void*>(hook_game_idle), &g_game_idle, "CGGameUI::Idle");
    patch_character_cap();
}

}  // namespace

void syn_net_notify(uint32_t a, uint32_t b, const char* text) {
    syn_net_notify_impl(a, b, text);
}

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
