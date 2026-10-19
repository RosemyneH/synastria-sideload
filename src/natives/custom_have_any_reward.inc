const uint8_t reward_plain[0x16] = {
    0, 0, 1, 0, 1, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0,
};

std::map<uint32_t, std::vector<uint16_t>> reward_items;
bool reward_ready = false;

struct RewardIn {
    const uint8_t* data;
    size_t size;
    size_t off;
    bool ok;

    bool take(void* dst, size_t n) {
        if (!ok || off + n > size) {
            ok = false;
            return false;
        }
        memcpy(dst, data + off, n);
        off += n;
        return true;
    }

    uint8_t u8() {
        uint8_t value = 0;
        take(&value, 1);
        return value;
    }

    uint16_t u16() {
        uint16_t value = 0;
        take(&value, 2);
        return value;
    }

    uint32_t u32() {
        uint32_t value = 0;
        take(&value, 4);
        return value;
    }

    uint32_t varint() {
        uint8_t byte = u8();
        uint32_t value = byte & 0x7fu;
        for (int left = 4; (byte & 0x80) != 0 && left > 0 && ok; --left) {
            byte = u8();
            value = (byte & 0x7fu) | (value << 7);
        }
        return value;
    }

    void cstr() {
        while (ok) {
            if (u8() == 0) {
                break;
            }
        }
    }

    void u16_list() {
        uint8_t count = u8();
        for (uint8_t i = 0; i < count && ok; ++i) {
            u16();
        }
    }

    void extra() {
        uint32_t flags = u32();
        if (flags & 0x1) {
            u16();
        }
        if (flags & 0x8) {
            u16();
        }
        if (flags & 0x10) {
            u16();
        }
        if (flags & 0x20) {
            u8();
        }
        if (flags & 0x40) {
            u8();
        }
        if (flags & 0x80) {
            u16();
            u16();
        }
        if (flags & 0x100) {
            u32();
        }
        if (flags & 0x200) {
            u32();
        }
        if (flags & 0x400) {
            u32();
        }
        if (flags & 0x800) {
            u16_list();
        }
        if (flags & 0x1000) {
            u16();
        }
        if (flags & 0x2000) {
            u16_list();
        }
        if (flags & 0x100000) {
            u16_list();
        }
        if (flags & 0x200000) {
            u16_list();
        }
        if (flags & 0x400000) {
            u16_list();
        }
        if (flags & 0x800000) {
            u16_list();
        }
    }
};

void load_have_any_reward(const uint8_t* data, size_t size) {
    reward_ready = true;
    reward_items.clear();
    if (!data || size < 8) {
        return;
    }
    RewardIn in{data, size, 0, true};
    uint32_t version = in.u32();
    in.u32();
    if (version < 1 || version > 5) {
        return;
    }
    uint8_t creatures[0x2000];
    if (!in.take(creatures, sizeof(creatures))) {
        return;
    }
    for (int id = 0; id < 0x10000 && in.ok; ++id) {
        if ((creatures[id >> 3] & (1u << (id & 7))) == 0) {
            continue;
        }
        in.cstr();
        if (in.u16() != 0xffff) {
            in.u8();
        }
        if (version >= 2) {
            uint8_t flags = in.u8();
            for (int bit = 0; bit < 3; ++bit) {
                if (flags & (1 << bit)) {
                    in.u16();
                }
            }
        }
    }
    uint32_t objects = in.u32();
    for (uint32_t i = 0; i < objects && in.ok; ++i) {
        in.varint();
        in.cstr();
        if (in.u16() != 0xffff) {
            in.u8();
        }
    }
    uint32_t quests = in.u32();
    if (version >= 5) {
        in.u32();
    }
    std::map<uint32_t, uint8_t> known;
    uint32_t running = 0;
    for (uint32_t i = 0; i < quests && in.ok; ++i) {
        running += in.varint();
        in.cstr();
        in.u16();
        if (version >= 3) {
            uint8_t flags = in.u8();
            if (flags & 0x04) {
                in.extra();
            }
        }
        known[running] = 1;
    }
    uint8_t items[0x2000];
    if (!in.take(items, sizeof(items))) {
        reward_items.clear();
        return;
    }
    for (int item = 0; item < 0x10000 && in.ok; ++item) {
        if ((items[item >> 3] & (1u << (item & 7))) == 0) {
            continue;
        }
        uint8_t kind = in.u8();
        while (kind != 0xff && in.ok) {
            uint32_t group = in.varint();
            bool extra = kind <= 0x15 && reward_plain[kind] == 0;
            uint32_t source = 0;
            for (uint32_t n = 0; n < group && in.ok; ++n) {
                source += in.varint();
                if (extra) {
                    in.u16();
                    in.u16();
                }
                if (kind == 2 && item != 0 && known.find(source) != known.end()) {
                    reward_items[source].push_back(static_cast<uint16_t>(item));
                }
            }
            kind = in.u8();
        }
    }
    if (!in.ok) {
        reward_items.clear();
        log_line("HaveAnyReward itemloc rejected");
        return;
    }
    log_line("HaveAnyReward quests %zu", reward_items.size());
}

void reward_ensure() {
    if (reward_ready || !g_root[0]) {
        return;
    }
    reward_ready = true;
    char path[1024];
    snprintf(path, sizeof(path), "%s/WoWExtItemLoc.bin", g_root);
    size_t size = 0;
    uint8_t* bytes = read_alloc(path, &size);
    if (!bytes) {
        return;
    }
    load_have_any_reward(bytes, size);
    free(bytes);
}

using RewardActiveFn = uint64_t (*)();
using RewardObjectFn = void* (*)(uint64_t guid, uint32_t mask, const char* file, int line);
using RewardBagFn = void* (*)(void* player);
using RewardSlotFn = void* (*)(void* bag, int slot);
using RewardCountFn = int (*)(void* bag, int item, uint32_t flags);

struct RewardApi {
    RewardActiveFn active = nullptr;
    RewardObjectFn object = nullptr;
    RewardBagFn bag = nullptr;
    RewardSlotFn slot = nullptr;
    RewardCountFn count = nullptr;
    bool bound = false;
};

RewardApi& reward_api() {
    static RewardApi api;
    if (api.bound) {
        return api;
    }
    api.bound = true;
    void* found = nullptr;
    if (syn_resolve("_Z25ClntObjMgrGetActivePlayerv", &found)) {
        api.active = reinterpret_cast<RewardActiveFn>(found);
    }
    if (syn_resolve("_Z19ClntObjMgrObjectPtr7WOWGUID8TypeMaskPKci", &found)) {
        api.object = reinterpret_cast<RewardObjectFn>(found);
    }
    if (syn_resolve("_ZNK10CGPlayer_C6GetBagEv", &found)) {
        api.bag = reinterpret_cast<RewardBagFn>(found);
    }
    if (syn_resolve("_ZNK7CGBag_C14GetItemPointerEi", &found)) {
        api.slot = reinterpret_cast<RewardSlotFn>(found);
    }
    if (syn_resolve("_ZN7CGBag_C16GetItemTypeCountEij", &found)) {
        api.count = reinterpret_cast<RewardCountFn>(found);
    }
    return api;
}

// ʕ •ᴥ•ʔ✿ m_obj +0x10, entry at +0xc; m_item +0x168, stack at +0x20 ✿ ʕ •ᴥ•ʔ
int reward_entry_stack(void* item, int want) {
    void* fields = nullptr;
    memcpy(&fields, static_cast<char*>(item) + 0x10, sizeof(fields));
    if (!fields) {
        return 0;
    }
    int32_t entry = 0;
    memcpy(&entry, static_cast<char*>(fields) + 0xc, sizeof(entry));
    if (entry != want) {
        return 0;
    }
    void* data = nullptr;
    memcpy(&data, static_cast<char*>(item) + 0x168, sizeof(data));
    if (!data) {
        return 0;
    }
    int32_t stack = 0;
    memcpy(&stack, static_cast<char*>(data) + 0x20, sizeof(stack));
    return stack;
}

int reward_held(int item) {
    RewardApi& api = reward_api();
    int total = 0;
    void* player = nullptr;
    if (api.active && api.object) {
        player = api.object(api.active(), 0x10, "test:1", 0xa0);
    }
    void* bag = player && api.bag ? api.bag(player) : nullptr;
    if (bag && item > 0 && api.count) {
        total = api.count(bag, item, 8);
    }
    double extra = game_num(13, item);
    if (extra >= 1.0 && extra < 2147483647.0) {
        total += static_cast<int>(extra);
    }
    if (bag && api.slot) {
        for (int slot = 0; slot < 0x13; ++slot) {
            void* found = api.slot(bag, slot);
            if (found) {
                total += reward_entry_stack(found, item);
            }
        }
    }
    return total;
}

int reward_quest_id(double value) {
    if (!(value > -2147483648.0)) {
        return 0;
    }
    if (!(value < 2147483647.0)) {
        return 0x7fffffff;
    }
    if (value < 1.0) {
        return 0;
    }
    return static_cast<int>(value);
}

bool reward_marked_only(double value) {
    if (!(value > -2147483648.0)) {
        return false;
    }
    if (!(value < 2147483647.0)) {
        return true;
    }
    return value >= 1.0;
}

// ʕ •ᴥ•ʔ✿ One quest id, optional second number. Pushes 1 when a quest reward of that id is held. ✿ ʕ •ᴥ•ʔ
int native_have_any_reward(void* state) {
    reward_ensure();
    if (!g_is_num || !g_to_num || !g_push_num || !g_is_num(state, 1)) {
        return 0;
    }
    int quest = reward_quest_id(g_to_num(state, 1));
    if (quest <= 0) {
        return 0;
    }
    auto found = reward_items.find(static_cast<uint32_t>(quest));
    if (found == reward_items.end() || found->second.empty()) {
        return 0;
    }
    bool marked = g_is_num(state, 2) && reward_marked_only(g_to_num(state, 2));
    for (uint16_t item : found->second) {
        const ItemRow* row = item_row(item);
        if (!row) {
            continue;
        }
        if (marked && (row->tag_word & 0x40) == 0) {
            continue;
        }
        if (reward_held(item) > 0) {
            g_push_num(state, 1.0);
            return 1;
        }
    }
    return 0;
}
