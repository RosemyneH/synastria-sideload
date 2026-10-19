struct QuestReqRow {
    std::vector<uint16_t> list[3];
};

std::map<int, QuestReqRow> quest_req_rows;
bool quest_req_ready = false;

using QuestReqCreateFn = void (*)(void*, int, int);
using QuestReqSetFn = void (*)(void*, int);
using QuestReqCheckFn = int (*)(void*, int);

QuestReqCreateFn quest_req_create = nullptr;
QuestReqSetFn quest_req_set = nullptr;
QuestReqCheckFn quest_req_check = nullptr;
bool quest_req_lua = false;

uint32_t quest_req_varint(BinReader& in) {
    uint32_t value = 0;
    uint8_t byte = 0;
    do {
        byte = in.u8();
        value = (value << 7) | (byte & 0x7f);
    } while (in.ok && (byte & 0x80) != 0);
    return value;
}

void quest_req_skip_cstr(BinReader& in) {
    while (in.ok) {
        if (in.u8() == 0) {
            break;
        }
    }
}

void quest_req_skip_tail(BinReader& in) {
    if (in.u16() != 0xffff) {
        in.u8();
    }
}

void quest_req_skip_list(BinReader& in) {
    uint8_t count = in.u8();
    for (uint8_t i = 0; i < count && in.ok; ++i) {
        in.u16();
    }
}

std::vector<uint16_t> quest_req_take_list(BinReader& in) {
    std::vector<uint16_t> words;
    uint8_t count = in.u8();
    words.reserve(count);
    for (uint8_t i = 0; i < count && in.ok; ++i) {
        words.push_back(in.u16());
    }
    return words;
}

void quest_req_extra(BinReader& in, QuestReqRow& row) {
    uint32_t flags = in.u32();
    if (flags & 0x1) {
        in.u16();
    }
    if (flags & 0x8) {
        in.u16();
    }
    if (flags & 0x10) {
        in.u16();
    }
    if (flags & 0x20) {
        in.u8();
    }
    if (flags & 0x40) {
        in.u8();
    }
    if (flags & 0x80) {
        in.u16();
        in.u16();
    }
    if (flags & 0x100) {
        in.u32();
    }
    if (flags & 0x200) {
        in.u32();
    }
    if (flags & 0x400) {
        in.u32();
    }
    if (flags & 0x800) {
        quest_req_skip_list(in);
    }
    if (flags & 0x1000) {
        in.u16();
    }
    if (flags & 0x2000) {
        quest_req_skip_list(in);
    }
    if (flags & 0x100000) {
        quest_req_skip_list(in);
    }
    if (flags & 0x200000) {
        row.list[0] = quest_req_take_list(in);
    }
    if (flags & 0x400000) {
        row.list[1] = quest_req_take_list(in);
    }
    if (flags & 0x800000) {
        row.list[2] = quest_req_take_list(in);
    }
}

void load_quest_req(const uint8_t* data, size_t size) {
    quest_req_rows.clear();
    quest_req_ready = true;
    if (!data || size < 8) {
        return;
    }
    BinReader in{data, size, 0, true};
    uint32_t version = in.u32();
    in.u32();
    if (version < 1 || version > 5) {
        return;
    }
    uint8_t bitmap[0x2000];
    if (!in.take(bitmap, sizeof(bitmap))) {
        return;
    }
    for (int id = 0; id < 0x10000 && in.ok; ++id) {
        if ((bitmap[id >> 3] & (1 << (id & 7))) == 0) {
            continue;
        }
        quest_req_skip_cstr(in);
        quest_req_skip_tail(in);
        if (version >= 2) {
            uint8_t flags = in.u8();
            for (int bit = 0; bit < 3; ++bit) {
                if (flags & (1 << bit)) {
                    in.u16();
                }
            }
        }
    }
    uint32_t zones = in.u32();
    for (uint32_t i = 0; i < zones && in.ok; ++i) {
        quest_req_varint(in);
        quest_req_skip_cstr(in);
        quest_req_skip_tail(in);
    }
    uint32_t quests = in.u32();
    if (version >= 5) {
        in.u32();
    }
    uint32_t running = 0;
    for (uint32_t i = 0; i < quests && in.ok; ++i) {
        running += quest_req_varint(in);
        quest_req_skip_cstr(in);
        in.u16();
        if (version < 3) {
            continue;
        }
        uint8_t mark = in.u8();
        if ((mark & 0x04) == 0) {
            continue;
        }
        QuestReqRow row;
        quest_req_extra(in, row);
        if (running == 0 || running >= 0x10000) {
            continue;
        }
        if (!row.list[0].empty() || !row.list[1].empty() || !row.list[2].empty()) {
            quest_req_rows[static_cast<int>(running)] = std::move(row);
        }
    }
    log_line("WoWExtItemLoc.bin quest reqs %zu", quest_req_rows.size());
}

void quest_req_ensure() {
    if (quest_req_ready || !g_root[0]) {
        return;
    }
    char path[1024];
    snprintf(path, sizeof(path), "%s/WoWExtItemLoc.bin", g_root);
    size_t size = 0;
    uint8_t* bytes = read_alloc(path, &size);
    if (!bytes) {
        quest_req_ready = true;
        return;
    }
    load_quest_req(bytes, size);
    free(bytes);
}

void quest_req_bind() {
    if (quest_req_lua) {
        return;
    }
    quest_req_lua = true;
    syn_resolve("lua_createtable", reinterpret_cast<void**>(&quest_req_create));
    syn_resolve("lua_settable", reinterpret_cast<void**>(&quest_req_set));
    syn_resolve("lua_checkstack", reinterpret_cast<void**>(&quest_req_check));
}

int quest_req_arg(void* state, int index, int* out) {
    if (!g_is_num || !g_to_num || !g_is_num(state, index)) {
        return 0;
    }
    double value = g_to_num(state, index);
    if (!(value > -2147483648.0) || !(value < 2147483647.0)) {
        *out = value >= 0.0 ? 0x7fffffff : static_cast<int>(0x80000000u);
        return 1;
    }
    double rounded = value >= 0.0 ? value + 0.5 : value - 0.5;
    if (!(rounded > -2147483648.0) || !(rounded < 2147483647.0)) {
        *out = rounded >= 0.0 ? 0x7fffffff : static_cast<int>(0x80000000u);
        return 1;
    }
    *out = static_cast<int>(rounded);
    return 1;
}

// ʕ •ᴥ•ʔ✿ kind 1/2/3 are extra flags 0x200000, 0x400000, 0x800000. Missing data is an empty table. ✿ ʕ •ᴥ•ʔ
int native_get_quest_req(void* state) {
    quest_req_ensure();
    quest_req_bind();
    int quest = 0;
    int kind = 0;
    if (!quest_req_arg(state, 1, &quest) || !quest_req_arg(state, 2, &kind)) {
        return 0;
    }
    if (quest <= 0 || quest >= 0x10000 || kind < 1 || kind > 3) {
        return 0;
    }
    if (!quest_req_create || !quest_req_set || !g_push_num) {
        return 0;
    }
    quest_req_create(state, 0, 0);
    auto found = quest_req_rows.find(quest);
    if (found == quest_req_rows.end()) {
        return 1;
    }
    const std::vector<uint16_t>& words = found->second.list[kind - 1];
    int index = 1;
    for (uint16_t word : words) {
        if (word == 0) {
            break;
        }
        if (quest_req_check) {
            quest_req_check(state, 2);
        }
        g_push_num(state, index);
        g_push_num(state, word);
        quest_req_set(state, -3);
        ++index;
    }
    return 1;
}
