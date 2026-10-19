bool have_item_id(double value, int* out) {
    constexpr double k_min = -2147483648.0;
    constexpr double k_max = 2147483647.0;
    if (!(value > k_min)) {
        return false;
    }
    if (!(value < k_max)) {
        *out = 0x7fffffff;
        return true;
    }
    double rounded = value >= 0.0
        ? static_cast<double>(static_cast<long long>(value + 0.5))
        : static_cast<double>(static_cast<long long>(value - 0.5));
    if (!(rounded > k_min)) {
        return false;
    }
    if (!(rounded < k_max)) {
        *out = 0x7fffffff;
        return true;
    }
    int id = static_cast<int>(rounded);
    if (id <= 0) {
        return false;
    }
    *out = id;
    return true;
}

// ʕ •ᴥ•ʔ✿ item id to stack total; Custom_CacheHaveItems fills this ✿ ʕ •ᴥ•ʔ
std::map<int, int> g_have_items;

int native_is_have_item(void* state) {
    if (!g_is_num || !g_to_num || !g_push_num || !g_is_num(state, 1)) {
        return 0;
    }
    int id = 0;
    if (!have_item_id(g_to_num(state, 1), &id)) {
        return 0;
    }
    auto found = g_have_items.find(id);
    if (found == g_have_items.end()) {
        return 0;
    }
    g_push_num(state, found->second);
    return 1;
}
