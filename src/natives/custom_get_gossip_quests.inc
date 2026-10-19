struct GossipQuestSlot {
    uint32_t id;
    int32_t level;
    uint32_t flags;
    uint32_t repeatable;
    uint32_t icon;
    char title[512];
};

GossipQuestSlot* gossip_quests = nullptr;
int32_t* gossip_current_quest = nullptr;

using GossipTableFn = void (*)(void*, int, int);
using GossipRawFn = void (*)(void*, int);
using GossipCheckFn = int (*)(void*, int);

GossipTableFn gossip_table = nullptr;
GossipRawFn gossip_rawset = nullptr;
GossipCheckFn gossip_check = nullptr;
bool gossip_bound = false;

void gossip_bind() {
    if (gossip_bound) {
        return;
    }
    gossip_bound = true;
    void* found = nullptr;
    if (syn_resolve("_ZN12CGGossipInfo8m_questsE", &found)) {
        gossip_quests = static_cast<GossipQuestSlot*>(found);
    }
    found = nullptr;
    if (syn_resolve("_ZN11CGQuestInfo14m_currentQuestE", &found)) {
        gossip_current_quest = static_cast<int32_t*>(found);
    }
    found = nullptr;
    if (syn_resolve("lua_createtable", &found)) {
        gossip_table = reinterpret_cast<GossipTableFn>(found);
    }
    found = nullptr;
    if (syn_resolve("lua_rawset", &found)) {
        gossip_rawset = reinterpret_cast<GossipRawFn>(found);
    }
    found = nullptr;
    if (syn_resolve("lua_checkstack", &found)) {
        gossip_check = reinterpret_cast<GossipCheckFn>(found);
    }
}

int gossip_mode(void* state) {
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
    return static_cast<int>(rounded);
}

bool gossip_is_active(uint32_t icon) {
    return icon == 3 || icon == 4;
}

int gossip_count(bool active) {
    int count = 0;
    for (int i = 0; i < 32; ++i) {
        if (gossip_quests[i].id == 0) {
            break;
        }
        if (gossip_is_active(gossip_quests[i].icon) == active) {
            ++count;
        }
    }
    return count;
}

const GossipQuestSlot* gossip_at(bool active, int index) {
    int found = 0;
    for (int i = 0; i < 32; ++i) {
        if (gossip_quests[i].id == 0) {
            return nullptr;
        }
        if (gossip_is_active(gossip_quests[i].icon) != active) {
            continue;
        }
        if (found == index) {
            return &gossip_quests[i];
        }
        ++found;
    }
    return nullptr;
}

int gossip_push_ids(void* state, bool active) {
    int count = gossip_count(active);
    if (count <= 0) {
        return 0;
    }
    gossip_table(state, 0, 0);
    int slot = 1;
    for (int i = 0; i < count; ++i) {
        const GossipQuestSlot* quest = gossip_at(active, i);
        if (!quest) {
            continue;
        }
        if (gossip_check) {
            gossip_check(state, 2);
        }
        g_push_num(state, slot);
        g_push_num(state, static_cast<int32_t>(quest->id));
        gossip_rawset(state, -3);
        ++slot;
    }
    return 1;
}

// ʕ •ᴥ•ʔ✿ mode 1 available and mode 2 active are a 1-based id table from CGGossipInfo::m_quests; mode 3 is CGQuestInfo::m_currentQuest ✿ ʕ •ᴥ•ʔ
int native_get_gossip_quests(void* state) {
    int mode = gossip_mode(state);
    if (mode != 1 && mode != 2 && mode != 3) {
        return 0;
    }
    gossip_bind();
    if (mode == 3) {
        if (!gossip_current_quest || !g_push_num) {
            return 0;
        }
        g_push_num(state, *gossip_current_quest);
        return 1;
    }
    if (!gossip_quests || !gossip_table || !gossip_rawset || !g_push_num) {
        return 0;
    }
    return gossip_push_ids(state, mode == 2);
}
