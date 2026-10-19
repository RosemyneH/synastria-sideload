int extra_float_to_int(double value) {
    constexpr double k_min = -2147483648.0;
    constexpr double k_max = 2147483647.0;
    if (!(value > k_min)) {
        return static_cast<int>(0x80000000u);
    }
    if (!(value < k_max)) {
        return 0x7fffffff;
    }
    double rounded = value + (value >= 0.0 ? 0.5 : -0.5);
    if (!(rounded > k_min)) {
        return static_cast<int>(0x80000000u);
    }
    if (!(rounded < k_max)) {
        return 0x7fffffff;
    }
    return static_cast<int>(rounded);
}

// ʕ •ᴥ•ʔ✿ same entity_extra cell as GetInt: both dwords are one IEEE double ✿ ʕ •ᴥ•ʔ
double extra_float_lookup(int a, int b, int field, double fallback) {
    auto outer = entity_extra.find(extra_int_key(a, b));
    if (outer == entity_extra.end()) {
        return fallback;
    }
    auto inner = outer->second.find(field);
    if (inner == outer->second.end()) {
        return fallback;
    }
    uint64_t bits = static_cast<uint32_t>(inner->second.bits_lo)
        | (static_cast<uint64_t>(static_cast<uint32_t>(inner->second.bits_hi)) << 32);
    double value = 0.0;
    memcpy(&value, &bits, sizeof(value));
    return value;
}

int native_get_entity_extra_float(void* state) {
    if (!g_is_num || !g_to_num || !g_push_num) {
        return 0;
    }
    if (!g_is_num(state, 1) || !g_is_num(state, 2) || !g_is_num(state, 3)) {
        return 0;
    }
    int a = extra_float_to_int(g_to_num(state, 1));
    int b = extra_float_to_int(g_to_num(state, 2));
    int field = extra_float_to_int(g_to_num(state, 3));
    double fallback = 0.0;
    if (g_is_num(state, 4)) {
        fallback = g_to_num(state, 4);
    }
    g_push_num(state, extra_float_lookup(a, b, field, fallback));
    return 1;
}
