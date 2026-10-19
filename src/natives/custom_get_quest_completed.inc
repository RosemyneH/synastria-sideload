bool quest_done_span(double value) {
    return !(value <= -2147483648.0) && !(value >= 2147483647.0);
}

double quest_done_round(double value) {
    if (value != value) {
        return value;
    }
    if (value >= 0.0) {
        return static_cast<double>(static_cast<long long>(value + 0.5));
    }
    return static_cast<double>(static_cast<long long>(value - 0.5));
}

bool quest_done_has(int id) {
    static const uint8_t* list = nullptr;
    static bool bound = false;
    if (!bound) {
        bound = true;
        void* found = nullptr;
        if (syn_resolve("s_completedQuests", &found)) {
            list = static_cast<const uint8_t*>(found);
        }
    }
    if (!list) {
        return false;
    }
    const uint8_t* node = nullptr;
    memcpy(&node, list + 0x10, sizeof(node));
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

// ʕ •ᴥ•ʔ✿ One quest id. Pushes 1 when that id is in the completed set; otherwise nothing. ✿ ʕ •ᴥ•ʔ
int native_get_quest_completed(void* state) {
    if (!g_is_num || !g_to_num || !g_is_num(state, 1)) {
        return 0;
    }
    double value = g_to_num(state, 1);
    if (!quest_done_span(value)) {
        return 0;
    }
    double rounded = quest_done_round(value);
    if (!quest_done_span(rounded)) {
        return 0;
    }
    int id = rounded != rounded ? static_cast<int>(0x80000000u) : static_cast<int>(rounded);
    if (id <= 0 || id >= 0x10000) {
        return 0;
    }
    if (id >= 0x6d60 || !quest_done_has(id)) {
        return 0;
    }
    g_push_num(state, 1.0);
    return 1;
}
