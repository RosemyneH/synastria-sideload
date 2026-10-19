int32_t loot_rule_get_index(double value) {
    constexpr double k_min = -2147483648.0;
    constexpr double k_max = 2147483647.0;
    if (!(value > k_min)) {
        return 0x7fffffff;
    }
    if (!(value < k_max)) {
        return 0x7ffffffe;
    }
    double rounded = __builtin_round(value);
    if (!(rounded > k_min)) {
        return 0x7fffffff;
    }
    if (!(rounded < k_max)) {
        return 0x7ffffffe;
    }
    return static_cast<int32_t>(rounded) - 1;
}

// ʕ •ᴥ•ʔ✿ rule+0x24 is an x86 MSVC string; capacity lives at +0x14 and below 16 means the bytes are inline ✿ ʕ •ᴥ•ʔ
const char* loot_rule_get_name(const uint8_t* rule) {
    const uint8_t* text = rule + 0x24;
    uint32_t capacity = 0;
    memcpy(&capacity, text + 0x14, sizeof(capacity));
    if (capacity < 16) {
        return reinterpret_cast<const char*>(text);
    }
    uint32_t ptr = 0;
    memcpy(&ptr, text, sizeof(ptr));
    return reinterpret_cast<const char*>(static_cast<uintptr_t>(ptr));
}

int native_get_loot_filter_rule(void* state) {
    if (!loot_filter || !g_is_num || !g_to_num || !g_push_num || !g_push_str || !g_is_num(state, 1)) {
        return 0;
    }
    int32_t index = loot_rule_get_index(g_to_num(state, 1));
    if (index < 0) {
        return 0;
    }
    if (index >= static_cast<int>(loot_filter->rule_count)) {
        return 0;
    }
    const uint8_t* rule = loot_filter->rules[index];
    g_push_num(state, static_cast<double>(rule[0x20]));
    g_push_num(state, static_cast<double>(rule[0x21]));
    g_push_num(state, static_cast<double>(rule[0x22]));
    g_push_num(state, static_cast<double>(rule[0x23]));
    g_push_str(state, loot_rule_get_name(rule));
    return 5;
}
