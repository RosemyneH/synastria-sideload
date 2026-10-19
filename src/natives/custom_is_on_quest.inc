struct QuestLogRow {
    int32_t id;
    int32_t slot;
    int32_t header;
    int32_t expired;
};

QuestLogRow* quest_log_rows = nullptr;
int32_t* quest_log_count = nullptr;
bool quest_log_bound = false;

void bind_quest_log() {
    if (quest_log_bound) {
        return;
    }
    quest_log_bound = true;
    void* found = nullptr;
    if (syn_resolve("_ZN10CGQuestLog8m_questsE", &found)) {
        quest_log_rows = static_cast<QuestLogRow*>(found);
    }
    found = nullptr;
    if (syn_resolve("_ZN10CGQuestLog10m_numQuestE", &found)) {
        quest_log_count = static_cast<int32_t*>(found);
    }
}

// ʕ •ᴥ•ʔ✿ one quest id; round half away from 0; scan CGQuestLog::m_quests[0, m_numQuest) and push 1 on an id hit ✿ ʕ •ᴥ•ʔ
int native_is_on_quest(void* state) {
    if (!g_is_num || !g_to_num || !g_push_num || !g_is_num(state, 1)) {
        return 0;
    }
    double value = g_to_num(state, 1);
    constexpr double k_min = -2147483648.0;
    constexpr double k_max = 2147483647.0;
    if (!(value > k_min)) {
        return 0;
    }
    int id = 0x7fffffff;
    if (value < k_max) {
        double rounded = value >= 0.0 ? value + 0.5 : value - 0.5;
        if (!(rounded > k_min)) {
            return 0;
        }
        if (rounded < k_max) {
            id = static_cast<int>(rounded);
            if (id <= 0) {
                return 0;
            }
        }
    }
    bind_quest_log();
    if (!quest_log_rows || !quest_log_count || *quest_log_count <= 0) {
        return 0;
    }
    int count = *quest_log_count;
    if (count > 50) {
        count = 50;
    }
    for (int i = 0; i < count; ++i) {
        if (quest_log_rows[i].id == id) {
            g_push_num(state, 1.0);
            return 1;
        }
    }
    return 0;
}
