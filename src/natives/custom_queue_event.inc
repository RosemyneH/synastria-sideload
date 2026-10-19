int64_t pending_evt_arm = 0;

double queue_event_round(double value) {
    if (value != value) {
        return value;
    }
    double shifted = value >= 0.0 ? value + 0.5 : value - 0.5;
    return static_cast<double>(static_cast<long long>(shifted));
}

bool queue_event_id(double value, int* id) {
    constexpr double k_min = -2147483648.0;
    constexpr double k_max = 2147483647.0;
    if (!(value > k_min) || !(value < k_max)) {
        return false;
    }
    double rounded = queue_event_round(value);
    if (!(rounded > k_min) || !(rounded < k_max)) {
        return false;
    }
    int n = static_cast<int>(rounded);
    if (n <= 0 || n > 0xff) {
        return false;
    }
    *id = n;
    return true;
}

int queue_event_arg(double value) {
    constexpr double k_min = -2147483648.0;
    constexpr double k_max = 2147483647.0;
    if (!(value > k_min)) {
        return static_cast<int>(0x80000000u);
    }
    if (!(value < k_max)) {
        return 0x7fffffff;
    }
    double rounded = queue_event_round(value);
    if (!(rounded > k_min)) {
        return static_cast<int>(0x80000000u);
    }
    if (!(rounded < k_max)) {
        return 0x7fffffff;
    }
    return static_cast<int>(rounded);
}

// ʕ •ᴥ•ʔ✿ two numbers; id 1..255 and a saturated arg; returns nothing ✿ ʕ •ᴥ•ʔ
int native_queue_event(void* state) {
    if (!g_is_num || !g_to_num || !g_is_num(state, 1)) {
        return 0;
    }
    int id = 0;
    if (!queue_event_id(g_to_num(state, 1), &id)) {
        return 0;
    }
    int arg = 0;
    if (g_is_num(state, 2)) {
        arg = queue_event_arg(g_to_num(state, 2));
    }
    if (id == 0xf5) {
        if (arg == 0x395 && pending_evt_arm != 0x7fffffffffffffffLL) {
            pending_evt_arm = 0x7fffffffffffffffLL;
        }
        return 0;
    }
    uint32_t uarg = static_cast<uint32_t>(arg);
    uint32_t uid = static_cast<uint32_t>(id);
    if (!pending_evt_seen.insert(pending_evt_key(uarg, uid)).second) {
        return 0;
    }
    pending_evt_order.push_back(PendingEvtItem{uarg, uid});
    return 0;
}
