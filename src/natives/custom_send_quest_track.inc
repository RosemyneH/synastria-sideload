struct QuestTrackOp {
    uint8_t op;
    int32_t id;
};

std::vector<QuestTrackOp> quest_track_ops;

int quest_track_mode(double value) {
    constexpr double k_min = -2147483648.0;
    constexpr double k_max = 2147483647.0;
    if (value <= k_min) {
        return static_cast<int>(0x80000000u);
    }
    if (value >= k_max) {
        return 0x7fffffff;
    }
    if (value != value) {
        return static_cast<int>(0x80000000u);
    }
    double shifted = value >= 0.0 ? value + 0.5 : value - 0.5;
    double rounded = static_cast<double>(static_cast<long long>(shifted));
    if (rounded <= k_min) {
        return static_cast<int>(0x80000000u);
    }
    if (rounded >= k_max) {
        return 0x7fffffff;
    }
    return static_cast<int>(rounded);
}

bool quest_track_id(double value, int* id) {
    constexpr double k_min = -2147483648.0;
    constexpr double k_max = 2147483647.0;
    if (value <= k_min || value >= k_max || value != value) {
        return false;
    }
    double shifted = value >= 0.0 ? value + 0.5 : value - 0.5;
    double rounded = static_cast<double>(static_cast<long long>(shifted));
    if (rounded <= k_min || rounded >= k_max || rounded != rounded) {
        return false;
    }
    int n = static_cast<int>(rounded);
    if (n <= 0 || n > 0xffffff) {
        return false;
    }
    *id = n;
    return true;
}

void quest_track_apply(int mode, int id) {
    if ((mode != 1 && mode != 2) || id <= 0) {
        return;
    }
    uint8_t op = static_cast<uint8_t>(mode);
    for (size_t i = 0; i < quest_track_ops.size(); ++i) {
        if (quest_track_ops[i].id != id) {
            continue;
        }
        if (quest_track_ops[i].op == op) {
            return;
        }
        quest_track_ops.erase(quest_track_ops.begin() + static_cast<std::ptrdiff_t>(i));
        break;
    }
    quest_track_ops.push_back(QuestTrackOp{op, id});
}

// ʕ •ᴥ•ʔ✿ mode 3 replaces the queue with a clear; 1 appends an id and 2 moves it to a remove; returns nothing ✿ ʕ •ᴥ•ʔ
int native_send_quest_track(void* state) {
    if (!g_is_num || !g_to_num || !g_is_num(state, 1)) {
        return 0;
    }
    int mode = quest_track_mode(g_to_num(state, 1));
    if (mode == 3) {
        quest_track_ops.clear();
        quest_track_ops.push_back(QuestTrackOp{3, 0});
        return 0;
    }
    int id = 0;
    if (!g_is_num(state, 2) || !quest_track_id(g_to_num(state, 2), &id)) {
        return 0;
    }
    quest_track_apply(mode, id);
    return 0;
}
