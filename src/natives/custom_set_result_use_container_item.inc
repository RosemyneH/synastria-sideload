// ʕ •ᴥ•ʔ✿ use-container callback zeros this, then treats a non-zero value as the handler result ✿ ʕ •ᴥ•ʔ
int32_t use_result_code = 0;

int32_t use_result_round(double value) {
    constexpr double k_min = -2147483648.0;
    constexpr double k_max = 2147483647.0;
    if (!(value > k_min)) {
        return static_cast<int32_t>(0x80000000u);
    }
    if (!(value < k_max)) {
        return 0x7fffffff;
    }
    double rounded = __builtin_round(value);
    if (!(rounded > k_min)) {
        return static_cast<int32_t>(0x80000000u);
    }
    if (!(rounded < k_max)) {
        return 0x7fffffff;
    }
    return static_cast<int32_t>(rounded);
}

int native_set_result_use_container_item(void* state) {
    if (!g_is_num || !g_to_num || !g_is_num(state, 1)) {
        return 0;
    }
    use_result_code = use_result_round(g_to_num(state, 1));
    return 0;
}
