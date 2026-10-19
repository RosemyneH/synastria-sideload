struct PrequestRow {
    uint32_t bits = 0;
    uint16_t class_mask = 0;
    uint16_t race_mask = 0;
    uint8_t min_level = 0;
    uint8_t max_level = 0;
    uint16_t skill = 0;
    uint16_t skill_need = 0;
    uint16_t rep_have = 0;
    uint16_t rep_lack = 0;
    uint16_t rep_cap = 0;
    int32_t rep_have_min = 0;
    int32_t rep_lack_max = 0;
    int32_t rep_cap_max = 0;
    bool has_all = false;
    bool has_any = false;
    bool has_excl = false;
    std::vector<uint16_t> all;
    std::vector<uint16_t> any;
    std::vector<uint16_t> excl;
};

std::map<int, PrequestRow> prequest_rows;
bool prequest_ready = false;

struct PrequestFind {
    std::map<uint16_t, uint8_t> ids;
    bool failed = false;
};

using PrequestActiveFn = uint64_t (*)();
using PrequestObjectFn = void* (*)(uint64_t, uint32_t, const char*, int);
using PrequestSkillFn = int (*)(void*, int);
using PrequestRepFn = int (*)(int);
using PrequestTableFn = void (*)(void*, int, int);
using PrequestRawFn = void (*)(void*, int);

PrequestActiveFn prequest_active = nullptr;
PrequestObjectFn prequest_object = nullptr;
PrequestSkillFn prequest_skill_index = nullptr;
PrequestRepFn prequest_rep = nullptr;
PrequestTableFn prequest_table = nullptr;
PrequestRawFn prequest_rawset = nullptr;

struct PrequestLogRow {
    int32_t id;
    int32_t slot;
    int32_t header;
    int32_t expired;
};

PrequestLogRow* prequest_log_rows = nullptr;
int32_t* prequest_log_count = nullptr;
const uint8_t* prequest_done_list = nullptr;
bool prequest_live_bound = false;
bool prequest_done_bound = false;

uint32_t prequest_varint(BinReader& in) {
    uint32_t value = 0;
    uint8_t byte = 0;
    do {
        byte = in.u8();
        value = (value << 7) | (byte & 0x7f);
    } while (in.ok && (byte & 0x80) != 0);
    return value;
}

void prequest_cstr(BinReader& in) {
    while (in.ok) {
        if (in.u8() == 0) {
            break;
        }
    }
}

void prequest_skip_named(BinReader& in, bool extra) {
    prequest_cstr(in);
    if (in.u16() != 0xffff) {
        in.u8();
    }
    if (!extra) {
        return;
    }
    uint8_t mask = in.u8();
    for (int bit = 0; bit < 3; ++bit) {
        if (mask & (1 << bit)) {
            in.u16();
        }
    }
}

void prequest_skip_words(BinReader& in) {
    uint8_t count = in.u8();
    for (uint8_t i = 0; i < count && in.ok; ++i) {
        in.u16();
    }
}

void prequest_take_words(BinReader& in, std::vector<uint16_t>& out) {
    uint8_t count = in.u8();
    out.reserve(count);
    for (uint8_t i = 0; i < count && in.ok; ++i) {
        out.push_back(in.u16());
    }
}

void prequest_scaled(BinReader& in, uint16_t& word, int32_t& scaled) {
    uint32_t packed = in.u32();
    word = static_cast<uint16_t>(packed);
    int32_t mag = static_cast<int32_t>((packed >> 16) & 0x7fff);
    scaled = (packed & 0x80000000u) ? -mag * 10 : mag * 10;
}

void load_prequests() {
    if (prequest_ready || !g_root[0]) {
        return;
    }
    prequest_ready = true;
    char path[1024];
    snprintf(path, sizeof(path), "%s/WoWExtItemLoc.bin", g_root);
    size_t size = 0;
    uint8_t* bytes = read_alloc(path, &size);
    if (!bytes) {
        return;
    }
    BinReader in{bytes, size, 0, true};
    uint32_t version = in.u32();
    in.u32();
    if (version < 1 || version > 5) {
        free(bytes);
        return;
    }
    uint8_t bitmap[0x2000];
    if (!in.take(bitmap, sizeof(bitmap))) {
        free(bytes);
        return;
    }
    for (int id = 0; id < 0x10000 && in.ok; ++id) {
        if ((bitmap[id >> 3] & (1 << (id & 7))) == 0) {
            continue;
        }
        prequest_skip_named(in, version >= 2);
    }
    uint32_t objects = in.u32();
    for (uint32_t i = 0; i < objects && in.ok; ++i) {
        prequest_varint(in);
        prequest_skip_named(in, false);
    }
    uint32_t quests = in.u32();
    if (version >= 5) {
        in.u32();
    }
    uint32_t running = 0;
    for (uint32_t i = 0; i < quests && in.ok; ++i) {
        running += prequest_varint(in);
        prequest_cstr(in);
        in.u16();
        if (version < 3) {
            continue;
        }
        uint8_t flags = in.u8();
        if ((flags & 0x04) == 0) {
            continue;
        }
        PrequestRow row;
        row.bits = in.u32();
        if (row.bits & 0x1) {
            row.class_mask = in.u16();
        } else if (row.bits & 0x2) {
            row.class_mask = 0x44d;
        } else if (row.bits & 0x4) {
            row.class_mask = 0x2b2;
        }
        if (row.bits & 0x8) {
            row.race_mask = in.u16();
        }
        if (row.bits & 0x10) {
            in.u16();
        }
        if (row.bits & 0x20) {
            row.min_level = in.u8();
        }
        if (row.bits & 0x40) {
            row.max_level = in.u8();
        }
        if (row.bits & 0x80) {
            row.skill = in.u16();
            row.skill_need = in.u16();
        }
        if (row.bits & 0x100) {
            prequest_scaled(in, row.rep_have, row.rep_have_min);
        }
        if (row.bits & 0x200) {
            prequest_scaled(in, row.rep_lack, row.rep_lack_max);
        }
        if (row.bits & 0x400) {
            prequest_scaled(in, row.rep_cap, row.rep_cap_max);
        }
        if (row.bits & 0x800) {
            prequest_skip_words(in);
        }
        if (row.bits & 0x1000) {
            in.u16();
        }
        if (row.bits & 0x2000) {
            prequest_skip_words(in);
        }
        if (row.bits & 0x100000) {
            prequest_skip_words(in);
        }
        if (row.bits & 0x200000) {
            row.has_all = true;
            prequest_take_words(in, row.all);
        }
        if (row.bits & 0x400000) {
            row.has_any = true;
            prequest_take_words(in, row.any);
        }
        if (row.bits & 0x800000) {
            row.has_excl = true;
            prequest_take_words(in, row.excl);
        }
        if (running > 0 && running < 0x10000) {
            prequest_rows[static_cast<int>(running)] = std::move(row);
        }
    }
    log_line("WoWExtItemLoc.bin prequests %zu", prequest_rows.size());
    free(bytes);
}

const PrequestRow* prequest_row(int id) {
    auto found = prequest_rows.find(id);
    if (found == prequest_rows.end()) {
        return nullptr;
    }
    return &found->second;
}

void prequest_bind_live() {
    if (prequest_live_bound) {
        return;
    }
    prequest_live_bound = true;
    void* found = nullptr;
    if (syn_resolve("_Z25ClntObjMgrGetActivePlayerv", &found)) {
        prequest_active = reinterpret_cast<PrequestActiveFn>(found);
    }
    found = nullptr;
    if (syn_resolve("_Z19ClntObjMgrObjectPtr7WOWGUID8TypeMaskPKci", &found)) {
        prequest_object = reinterpret_cast<PrequestObjectFn>(found);
    }
    found = nullptr;
    if (syn_resolve("_ZNK10CGPlayer_C13GetSkillIndexEi", &found)) {
        prequest_skill_index = reinterpret_cast<PrequestSkillFn>(found);
    }
    found = nullptr;
    if (syn_resolve("_Z18GetRepListRepValuei", &found)) {
        prequest_rep = reinterpret_cast<PrequestRepFn>(found);
    }
    found = nullptr;
    if (syn_resolve("lua_createtable", &found)) {
        prequest_table = reinterpret_cast<PrequestTableFn>(found);
    }
    found = nullptr;
    if (syn_resolve("lua_rawset", &found)) {
        prequest_rawset = reinterpret_cast<PrequestRawFn>(found);
    }
    found = nullptr;
    if (syn_resolve("_ZN10CGQuestLog8m_questsE", &found)) {
        prequest_log_rows = static_cast<PrequestLogRow*>(found);
    }
    found = nullptr;
    if (syn_resolve("_ZN10CGQuestLog10m_numQuestE", &found)) {
        prequest_log_count = static_cast<int32_t*>(found);
    }
}

bool prequest_have_done() {
    if (!prequest_done_bound) {
        prequest_done_bound = true;
        void* found = nullptr;
        if (syn_resolve("s_completedQuests", &found)) {
            prequest_done_list = static_cast<const uint8_t*>(found);
        }
    }
    return prequest_done_list != nullptr;
}

bool prequest_done(int id) {
    if (id < 0 || id >= 0x6d60 || !prequest_have_done()) {
        return false;
    }
    const uint8_t* node = nullptr;
    memcpy(&node, prequest_done_list + 0x10, sizeof(node));
    while (node && (reinterpret_cast<uintptr_t>(node) & 1) == 0) {
        int32_t quest = 0;
        memcpy(&quest, node + 0x10, sizeof(quest));
        if (quest == id) {
            return true;
        }
        const uint8_t* next = nullptr;
        memcpy(&next, node + 0x8, sizeof(next));
        node = next;
    }
    return false;
}

bool prequest_on_log(int id) {
    prequest_bind_live();
    if (!prequest_log_rows || !prequest_log_count || *prequest_log_count <= 0 || id == 0) {
        return false;
    }
    int count = *prequest_log_count;
    if (count > 50) {
        count = 50;
    }
    for (int i = 0; i < count; ++i) {
        if (prequest_log_rows[i].id == id) {
            return true;
        }
    }
    return false;
}

void* prequest_player() {
    prequest_bind_live();
    if (!prequest_active || !prequest_object) {
        return nullptr;
    }
    return prequest_object(prequest_active(), 0x10, "test:1", 0xa0);
}

// ʕ •ᴥ•ʔ✿ Linux player: unit +0x160, level +0xc0, race +0x44, class +0x45; skill rank at data +0x1890 ✿ ʕ •ᴥ•ʔ
void prequest_player_face(int* level, int* race, int* klass) {
    *level = 0;
    *race = 0;
    *klass = 0;
    void* player = prequest_player();
    if (!player) {
        return;
    }
    const uint8_t* unit = nullptr;
    memcpy(&unit, static_cast<uint8_t*>(player) + 0x160, sizeof(unit));
    if (!unit) {
        return;
    }
    memcpy(level, unit + 0xc0, 4);
    *race = unit[0x44];
    *klass = unit[0x45];
}

int prequest_mask_bit(int id) {
    if (id <= 0) {
        return 0;
    }
    return 1 << ((id - 1) & 31);
}

int prequest_skill_rank(int skill) {
    void* player = prequest_player();
    if (!player || !prequest_skill_index || skill == 0) {
        return 0;
    }
    int index = prequest_skill_index(player, skill);
    if (index < 0) {
        return 0;
    }
    const uint8_t* data = nullptr;
    memcpy(&data, static_cast<uint8_t*>(player) + 0x1890, sizeof(data));
    if (!data) {
        return 0;
    }
    uint16_t rank = 0;
    memcpy(&rank, data + static_cast<size_t>(index) * 12 + 0x7a4, 2);
    return rank;
}

int prequest_rep_value(int faction) {
    prequest_bind_live();
    if (!prequest_rep || faction == 0) {
        return 0;
    }
    return prequest_rep(faction);
}

int prequest_status(int id) {
    const PrequestRow* row = prequest_row(id);
    if (!row) {
        return -1;
    }
    int level = 0;
    int race = 0;
    int klass = 0;
    prequest_player_face(&level, &race, &klass);
    if (row->class_mask != 0 && (prequest_mask_bit(klass) & row->class_mask) == 0) {
        return -6;
    }
    if (row->race_mask != 0 && (prequest_mask_bit(race) & row->race_mask) == 0) {
        return -7;
    }
    if (row->min_level > 1 || row->max_level != 0) {
        if (row->max_level != 0 && level > row->max_level) {
            return -8;
        }
        if (row->min_level > 1 && level < row->min_level) {
            return -9;
        }
    }
    if (row->skill != 0 && prequest_skill_rank(row->skill) < row->skill_need) {
        return -10;
    }
    if (row->has_excl && prequest_have_done()) {
        for (uint16_t word : row->excl) {
            if (word == 0) {
                break;
            }
            uint16_t need = word & 0x7fff;
            if ((word & 0x8000) && need && prequest_on_log(need)) {
                return -12;
            }
            if (need < 0x6d60 && prequest_done(need)) {
                return -11;
            }
        }
    }
    if (row->rep_have != 0 && prequest_rep_value(row->rep_have) < row->rep_have_min) {
        return -13;
    }
    if (row->rep_lack != 0 && prequest_rep_value(row->rep_lack) >= row->rep_lack_max) {
        return -14;
    }
    if (row->rep_cap != 0 && prequest_rep_value(row->rep_cap) >= row->rep_cap_max) {
        return -15;
    }
    if (row->bits & 0x40000) {
        return -21;
    }
    if (row->has_all && prequest_have_done() && !row->all.empty() && row->all[0] != 0) {
        for (uint16_t word : row->all) {
            if (word == 0) {
                break;
            }
            uint16_t need = word & 0x7fff;
            if (need < 0x6d60 && prequest_done(need)) {
                continue;
            }
            if ((word & 0x8000) && need && prequest_on_log(need)) {
                continue;
            }
            return -2;
        }
    }
    if (!row->has_any || !prequest_have_done()) {
        return 1;
    }
    if (row->any.empty() || row->any[0] == 0) {
        return -2;
    }
    for (uint16_t word : row->any) {
        if (word == 0) {
            break;
        }
        uint16_t need = word & 0x7fff;
        if (need < 0x6d60 && prequest_done(need)) {
            return 1;
        }
        if ((word & 0x8000) && need && prequest_on_log(need)) {
            return 1;
        }
    }
    return -2;
}

void prequest_walk(int quest, PrequestFind& out) {
    const PrequestRow* row = prequest_row(quest);
    if (!row || out.failed) {
        return;
    }
    if (row->has_all) {
        for (uint16_t word : row->all) {
            if (word == 0 || out.failed) {
                break;
            }
            uint16_t id = word & 0x7fff;
            if (id != 0 && prequest_on_log(id)) {
                continue;
            }
            if (prequest_done(id)) {
                continue;
            }
            int status = prequest_status(id);
            if (status > 0) {
                out.ids[id] = 1;
            } else if (status == -2) {
                prequest_walk(id, out);
            } else {
                out.failed = true;
            }
        }
        if (out.failed) {
            return;
        }
    }
    if (!row->has_any) {
        return;
    }
    for (uint16_t word : row->any) {
        if (word == 0) {
            break;
        }
        uint16_t id = word & 0x7fff;
        if (id != 0 && prequest_on_log(id)) {
            return;
        }
        if (prequest_done(id)) {
            return;
        }
    }
    if (row->any.empty() || row->any[0] == 0) {
        out.failed = true;
        return;
    }
    bool have = false;
    size_t best_size = 0;
    PrequestFind best;
    for (uint16_t word : row->any) {
        if (word == 0) {
            break;
        }
        uint16_t id = word & 0x7fff;
        int status = prequest_status(id);
        if (status > 0) {
            out.ids[id] = 1;
            return;
        }
        if (status != -2) {
            continue;
        }
        PrequestFind sub;
        prequest_walk(id, sub);
        if (sub.failed) {
            continue;
        }
        if (!have || sub.ids.size() < best_size) {
            best_size = sub.ids.size();
            best = std::move(sub);
            have = true;
        }
    }
    if (!have) {
        out.failed = true;
        return;
    }
    out.ids.insert(best.ids.begin(), best.ids.end());
}

int prequest_arg(void* state) {
    if (!g_is_num || !g_to_num || !g_is_num(state, 1)) {
        return 0;
    }
    double value = g_to_num(state, 1);
    if (!(value > -2147483648.0) || !(value < 2147483647.0)) {
        return 0;
    }
    double rounded = value >= 0.0 ? value + 0.5 : value - 0.5;
    if (!(rounded > -2147483648.0) || !(rounded < 2147483647.0)) {
        return 0;
    }
    int id = static_cast<int>(rounded);
    if (id <= 0 || id >= 0x8000) {
        return 0;
    }
    return id;
}

// ʕ •ᴥ•ʔ✿ One quest id. Pushes a 1-based table of quests still required, or an empty table; nothing if the id is unknown. ✿ ʕ •ᴥ•ʔ
int native_get_prequest(void* state) {
    load_prequests();
    int id = prequest_arg(state);
    if (id == 0 || !prequest_row(id)) {
        return 0;
    }
    prequest_bind_live();
    if (!prequest_table || !prequest_rawset || !g_set_top || !g_push_num) {
        return 0;
    }
    prequest_table(state, 0, 0);
    if (prequest_done(id) || prequest_on_log(id) || prequest_status(id) > 0) {
        return 1;
    }
    PrequestFind found;
    prequest_walk(id, found);
    if (found.failed) {
        return 1;
    }
    int index = 1;
    for (const auto& item : found.ids) {
        g_set_top(state, 2);
        g_push_num(state, index);
        g_push_num(state, item.first);
        prequest_rawset(state, -3);
        ++index;
    }
    return 1;
}
