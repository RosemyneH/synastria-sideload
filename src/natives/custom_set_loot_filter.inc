// ʕ •ᴥ•ʔ✿ 0 idle, 1 filter changed, 2 reset. Set marks 1. ✿ ʕ •ᴥ•ʔ
uint8_t loot_filter_dirty = 0;

int32_t loot_set_round(double value) {
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

// ʕ •ᴥ•ʔ✿ +0x20 dword cleared. Name is an MSVC string at +0x24; emptied only when size at +0x34 is nonzero. ✿ ʕ •ᴥ•ʔ
void loot_set_clear_rule(uint8_t* rule) {
    *reinterpret_cast<uint32_t*>(rule + 0x20) = 0;
    if (*reinterpret_cast<uint32_t*>(rule + 0x34) == 0) {
        return;
    }
    uint32_t cap = *reinterpret_cast<uint32_t*>(rule + 0x38);
    if (cap >= 0x10) {
        std::free(*reinterpret_cast<void**>(rule + 0x24));
    }
    std::memset(rule + 0x24, 0, 0x14);
    *reinterpret_cast<uint32_t*>(rule + 0x38) = 0x0f;
}

int native_set_loot_filter(void* state) {
    if (!loot_filter || !g_is_num || !g_to_num || !g_is_num(state, 1) || !g_is_num(state, 2)) {
        return 0;
    }
    int32_t count = loot_set_round(g_to_num(state, 1));
    uint16_t options = static_cast<uint16_t>(loot_set_round(g_to_num(state, 2)));
    if (count < 0 || count > 32) {
        return 0;
    }
    if (count < 32) {
        for (int32_t i = count; i < 32; ++i) {
            loot_set_clear_rule(loot_filter->rules[i]);
        }
    }
    loot_filter->rule_count = static_cast<uint8_t>(count);
    loot_filter->options = options;
    loot_filter_dirty = 1;
    return 0;
}
