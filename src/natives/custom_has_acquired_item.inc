int acquired_id(double n) {
    constexpr double k_min = -2147483648.0;
    constexpr double k_max = 2147483647.0;
    if (!(n > k_min)) {
        return static_cast<int>(0x80000000u);
    }
    if (!(n < k_max)) {
        return 0x7fffffff;
    }
    double rounded = n + (n >= 0.0 ? 0.5 : -0.5);
    if (!(rounded > k_min)) {
        return static_cast<int>(0x80000000u);
    }
    if (!(rounded < k_max)) {
        return 0x7fffffff;
    }
    return static_cast<int>(rounded);
}

// ʕ •ᴥ•ʔ✿ custom game data category 0x1c. A value above 0.5 pushes 1. ✿ ʕ •ᴥ•ʔ
int native_has_acquired_item(void* state) {
    if (!g_is_num || !g_to_num || !g_push_num || !g_is_num(state, 1)) {
        return 0;
    }
    if (!(game_num(0x1c, acquired_id(g_to_num(state, 1))) > 0.5)) {
        return 0;
    }
    g_push_num(state, 1.0);
    return 1;
}
