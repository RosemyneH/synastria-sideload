struct QuestAcceptList {
    bool on = false;
    std::vector<uint16_t> words;
};

struct QuestAcceptRow {
    uint32_t flags = 0;
    uint16_t race_mask = 0;
    uint16_t class_mask = 0;
    uint8_t min_level = 0;
    uint8_t max_level = 0;
    uint16_t skill_id = 0;
    uint16_t skill_need = 0;
    uint16_t rep_id[3] = {};
    int32_t rep_value[3] = {};
    QuestAcceptList forbid;
    QuestAcceptList need_all;
    QuestAcceptList need_any;
};

std::map<int, QuestAcceptRow> quest_accept_rows;
bool quest_accept_ready = false;

uint8_t* quest_accept_info = nullptr;
void* (*quest_accept_player)() = nullptr;
int32_t (*quest_accept_skill_index)(const void*, int32_t) = nullptr;
uint8_t* quest_accept_quests = nullptr;
int32_t* quest_accept_num_quest = nullptr;
uint8_t* quest_accept_faction = nullptr;
uint8_t* quest_accept_standings = nullptr;
uint8_t* quest_accept_done = nullptr;
bool quest_accept_bound = false;

uint32_t quest_accept_varint(BinReader& in) {
    uint32_t value = 0;
    uint8_t byte = 0;
    do {
        byte = in.u8();
        value = (value << 7) | (byte & 0x7f);
    } while (in.ok && (byte & 0x80) != 0);
    return value;
}

void quest_accept_cstr(BinReader& in) {
    while (in.ok) {
        uint8_t byte = in.u8();
        if (!in.ok || byte == 0) {
            break;
        }
    }
}

void quest_accept_name_tail(BinReader& in) {
    if (in.u16() != 0xffff) {
        in.u8();
    }
}

void quest_accept_skip_words(BinReader& in) {
    uint8_t count = in.u8();
    for (uint8_t i = 0; i < count && in.ok; ++i) {
        in.u16();
    }
}

QuestAcceptList quest_accept_words(BinReader& in) {
    QuestAcceptList list;
    uint8_t count = in.u8();
    if (count == 0) {
        return list;
    }
    list.on = true;
    for (uint8_t i = 0; i < count && in.ok; ++i) {
        uint16_t word = in.u16();
        if (word == 0) {
            for (++i; i < count && in.ok; ++i) {
                in.u16();
            }
            break;
        }
        list.words.push_back(word);
    }
    return list;
}

// ʕ •ᴥ•ʔ✿ packed rep: faction in the low half, magnitude in bits 16-30, bit 31 negates, times 10 ✿ ʕ •ᴥ•ʔ
void quest_accept_rep_pack(BinReader& in, uint16_t& faction, int32_t& value) {
    uint32_t packed = in.u32();
    faction = static_cast<uint16_t>(packed);
    int32_t magnitude = static_cast<int32_t>((packed >> 16) & 0x7fff);
    value = (packed & 0x80000000u) ? -10 * magnitude : 10 * magnitude;
}

// ʕ •ᴥ•ʔ✿ +4 race mask (0x44d alliance, 0x2b2 horde), +6 class mask ✿ ʕ •ᴥ•ʔ
QuestAcceptRow quest_accept_row(BinReader& in) {
    QuestAcceptRow row;
    row.flags = in.u32();
    if (row.flags & 0x1) {
        row.race_mask = in.u16();
    } else if (row.flags & 0x2) {
        row.race_mask = 0x44d;
    } else if (row.flags & 0x4) {
        row.race_mask = 0x2b2;
    }
    if (row.flags & 0x8) {
        row.class_mask = in.u16();
    }
    if (row.flags & 0x10) {
        in.u16();
    }
    if (row.flags & 0x20) {
        row.min_level = in.u8();
    }
    if (row.flags & 0x40) {
        row.max_level = in.u8();
    }
    if (row.flags & 0x80) {
        row.skill_id = in.u16();
        row.skill_need = in.u16();
    }
    if (row.flags & 0x100) {
        quest_accept_rep_pack(in, row.rep_id[0], row.rep_value[0]);
    }
    if (row.flags & 0x200) {
        quest_accept_rep_pack(in, row.rep_id[1], row.rep_value[1]);
    }
    if (row.flags & 0x400) {
        quest_accept_rep_pack(in, row.rep_id[2], row.rep_value[2]);
    }
    if (row.flags & 0x800) {
        quest_accept_skip_words(in);
    }
    if (row.flags & 0x1000) {
        in.u16();
    }
    if (row.flags & 0x2000) {
        quest_accept_skip_words(in);
    }
    if (row.flags & 0x100000) {
        quest_accept_skip_words(in);
    }
    if (row.flags & 0x200000) {
        row.need_all = quest_accept_words(in);
    }
    if (row.flags & 0x400000) {
        row.need_any = quest_accept_words(in);
    }
    if (row.flags & 0x800000) {
        row.forbid = quest_accept_words(in);
    }
    return row;
}

void load_quest_accept() {
    if (quest_accept_ready || !g_root[0]) {
        return;
    }
    quest_accept_ready = true;
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
        quest_accept_cstr(in);
        quest_accept_name_tail(in);
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
        quest_accept_varint(in);
        quest_accept_cstr(in);
        quest_accept_name_tail(in);
    }
    uint32_t quests = in.u32();
    if (version >= 5) {
        in.u32();
    }
    uint32_t running = 0;
    for (uint32_t i = 0; i < quests && in.ok; ++i) {
        running += quest_accept_varint(in);
        quest_accept_cstr(in);
        in.u16();
        if (version >= 3) {
            uint8_t flags = in.u8();
            if (flags & 0x04) {
                QuestAcceptRow row = quest_accept_row(in);
                if (in.ok && running > 0 && running < 0x10000) {
                    quest_accept_rows[static_cast<int>(running)] = std::move(row);
                }
            }
        }
    }
    log_line("WoWExtItemLoc.bin quest accept %zu", quest_accept_rows.size());
    free(bytes);
}

void quest_accept_bind() {
    if (quest_accept_bound) {
        return;
    }
    quest_accept_bound = true;
    void* found = nullptr;
    if (syn_resolve("_ZN14ClientServices15s_characterInfoE", &found)) {
        quest_accept_info = static_cast<uint8_t*>(found);
    }
    syn_resolve("_Z28ClntObjMgrGetActivePlayerObjv", reinterpret_cast<void**>(&quest_accept_player));
    syn_resolve("_ZNK10CGPlayer_C13GetSkillIndexEi", reinterpret_cast<void**>(&quest_accept_skill_index));
    if (syn_resolve("_ZN10CGQuestLog8m_questsE", &found)) {
        quest_accept_quests = static_cast<uint8_t*>(found);
    }
    if (syn_resolve("_ZN10CGQuestLog10m_numQuestE", &found)) {
        quest_accept_num_quest = static_cast<int32_t*>(found);
    }
    if (syn_resolve("g_factionDB", &found)) {
        quest_accept_faction = static_cast<uint8_t*>(found);
    }
    if (syn_resolve("_ZN16CGReputationInfo18m_factionStandingsE", &found)) {
        quest_accept_standings = static_cast<uint8_t*>(found);
    }
    if (syn_resolve("s_completedQuests", &found)) {
        quest_accept_done = static_cast<uint8_t*>(found);
    }
}

uint32_t quest_accept_bit(uint32_t id) {
    return id ? (1u << ((id - 1) & 31)) : 0;
}

int quest_accept_level() {
    if (!quest_accept_player) {
        return 0;
    }
    uint8_t* player = static_cast<uint8_t*>(quest_accept_player());
    if (!player) {
        return 0;
    }
    uint8_t* unit = nullptr;
    memcpy(&unit, player + 0x160, sizeof(unit));
    if (!unit) {
        return 0;
    }
    int32_t level = 0;
    memcpy(&level, unit + 0xc0, 4);
    return level;
}

int quest_accept_skill(int skill_id) {
    if (!quest_accept_player || !quest_accept_skill_index || skill_id == 0) {
        return 0;
    }
    const void* player = quest_accept_player();
    if (!player) {
        return 0;
    }
    int32_t index = quest_accept_skill_index(player, skill_id);
    if (index < 0) {
        return 0;
    }
    uint8_t* desc = nullptr;
    memcpy(&desc, static_cast<const uint8_t*>(player) + 0x1890, sizeof(desc));
    if (!desc) {
        return 0;
    }
    uint16_t rank = 0;
    memcpy(&rank, desc + 0x7a4 + static_cast<size_t>(index) * 12, 2);
    return rank;
}

int quest_accept_rep(int faction) {
    if (faction <= 0 || !quest_accept_faction || !quest_accept_standings) {
        return 0;
    }
    int32_t max_id = 0;
    int32_t min_id = 0;
    memcpy(&max_id, quest_accept_faction + 0x10, 4);
    memcpy(&min_id, quest_accept_faction + 0x14, 4);
    if (faction < min_id || faction > max_id) {
        return 0;
    }
    void** by_id = nullptr;
    memcpy(&by_id, quest_accept_faction + 0x30, sizeof(by_id));
    if (!by_id) {
        return 0;
    }
    const uint8_t* row = static_cast<const uint8_t*>(by_id[faction - min_id]);
    if (!row) {
        return 0;
    }
    int32_t index = 0;
    memcpy(&index, row + 4, 4);
    if (static_cast<uint32_t>(index) > 127) {
        return 0;
    }
    int32_t base = 0;
    int32_t standing = 0;
    const uint8_t* slot = quest_accept_standings + static_cast<size_t>(index) * 16;
    memcpy(&base, slot + 8, 4);
    memcpy(&standing, slot + 12, 4);
    return base + standing;
}

bool quest_accept_on_log(int id) {
    if (!quest_accept_quests || !quest_accept_num_quest || id <= 0) {
        return false;
    }
    int count = *quest_accept_num_quest;
    if (count > 50) {
        count = 50;
    }
    for (int i = 0; i < count; ++i) {
        int32_t quest = 0;
        memcpy(&quest, quest_accept_quests + static_cast<size_t>(i) * 16, 4);
        if (quest == id) {
            return true;
        }
    }
    return false;
}

bool quest_accept_completed(int id) {
    if (!quest_accept_done || id < 0 || id >= 0x6d60) {
        return false;
    }
    const uint8_t* node = nullptr;
    memcpy(&node, quest_accept_done + 0x10, sizeof(node));
    while (node && (reinterpret_cast<uintptr_t>(node) & 1) == 0) {
        int32_t quest = 0;
        memcpy(&quest, node + 0x10, 4);
        if (quest == id) {
            return true;
        }
        const uint8_t* next = nullptr;
        memcpy(&next, node + 8, sizeof(next));
        node = next;
    }
    return false;
}

// ʕ •ᴥ•ʔ✿ high bit: the quest log counts; the low 15 bits are the quest id ✿ ʕ •ᴥ•ʔ
int quest_accept_need_all(const QuestAcceptList& list) {
    if (!list.on || !quest_accept_done || list.words.empty()) {
        return 0;
    }
    for (uint16_t word : list.words) {
        int id = word & 0x7fff;
        int high = word >> 15;
        if (id < 0x6d60 && quest_accept_completed(id)) {
            continue;
        }
        if (high == 0 || id == 0 || !quest_accept_on_log(id)) {
            return -2;
        }
    }
    return 0;
}

int quest_accept_need_any(const QuestAcceptList& list) {
    if (!list.on || !quest_accept_done) {
        return 0;
    }
    if (list.words.empty()) {
        return -2;
    }
    for (uint16_t word : list.words) {
        int id = word & 0x7fff;
        int high = word >> 15;
        if (id < 0x6d60 && quest_accept_completed(id)) {
            return 0;
        }
        if (high && id && quest_accept_on_log(id)) {
            return 0;
        }
    }
    return -2;
}

int quest_accept_forbid(const QuestAcceptList& list) {
    if (!list.on || !quest_accept_done || list.words.empty()) {
        return 0;
    }
    for (uint16_t word : list.words) {
        int id = word & 0x7fff;
        int high = word >> 15;
        if (high && id && quest_accept_on_log(id)) {
            return -12;
        }
        if (id < 0x6d60 && quest_accept_completed(id)) {
            return -11;
        }
    }
    return 0;
}

int quest_accept_check(int id) {
    auto found = quest_accept_rows.find(id);
    if (found == quest_accept_rows.end()) {
        return -1;
    }
    const QuestAcceptRow& row = found->second;
    if (quest_accept_done && id < 0x6d60 && quest_accept_completed(id)) {
        return -3;
    }
    if (row.race_mask != 0) {
        uint32_t race = quest_accept_info ? quest_accept_info[0x178] : 0;
        if ((quest_accept_bit(race) & row.race_mask) == 0) {
            return -6;
        }
    }
    if (row.class_mask != 0) {
        uint32_t klass = quest_accept_info ? quest_accept_info[0x179] : 0;
        if ((quest_accept_bit(klass) & row.class_mask) == 0) {
            return -7;
        }
    }
    if (row.min_level > 1 || row.max_level != 0) {
        int level = quest_accept_level();
        if (row.max_level != 0 && level > row.max_level) {
            return -8;
        }
        if (row.min_level > 1 && level < row.min_level) {
            return -9;
        }
    }
    if (row.skill_id != 0 && quest_accept_skill(row.skill_id) < row.skill_need) {
        return -10;
    }
    if (int code = quest_accept_forbid(row.forbid)) {
        return code;
    }
    if (row.rep_id[0] != 0 && quest_accept_rep(row.rep_id[0]) < row.rep_value[0]) {
        return -13;
    }
    if (row.rep_id[1] != 0 && quest_accept_rep(row.rep_id[1]) >= row.rep_value[1]) {
        return -14;
    }
    if (row.rep_id[2] != 0 && quest_accept_rep(row.rep_id[2]) >= row.rep_value[2]) {
        return -15;
    }
    if (row.flags & 0x40000) {
        return -21;
    }
    if (int code = quest_accept_need_all(row.need_all)) {
        return code;
    }
    if (int code = quest_accept_need_any(row.need_any)) {
        return code;
    }
    return 1;
}

int quest_accept_round(double value) {
    constexpr double k_min = -2147483648.0;
    constexpr double k_max = 2147483647.0;
    if (!(value > k_min) || !(value < k_max)) {
        return 0;
    }
    double rounded = value >= 0.0 ? value + 0.5 : value - 0.5;
    if (!(rounded > k_min) || !(rounded < k_max)) {
        return 0;
    }
    int id = static_cast<int>(rounded);
    return id > 0 && id < 0x10000 ? id : 0;
}

// ʕ •ᴥ•ʔ✿ One quest id. Pushes the accept code, or -1 when the id is out of range. ✿ ʕ •ᴥ•ʔ
int native_get_quest_can_accept(void* state) {
    load_quest_accept();
    quest_accept_bind();
    if (!g_is_num || !g_to_num || !g_is_num(state, 1)) {
        return 0;
    }
    int id = quest_accept_round(g_to_num(state, 1));
    g_push_num(state, id > 0 ? quest_accept_check(id) : -1);
    return 1;
}
