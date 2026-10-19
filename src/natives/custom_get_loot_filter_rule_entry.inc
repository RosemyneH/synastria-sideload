int32_t loot_entry_get_index(double value) {
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

// ʕ •ᴥ•ʔ✿ entry is type, flags, u16. type 45 forces op 6 and type 46 forces op 7. flags < 0x20 stores op in the byte and a 15-bit sign-magnitude value; otherwise op is (flags>>5)-1 and the value uses flags bit4 plus bits 0-3. ✿ ʕ •ᴥ•ʔ
int loot_entry_get_op(uint8_t type, uint8_t flags) {
    if (type == 0x2d) {
        return 6;
    }
    int op = (flags >> 5) == 0 ? static_cast<int>(flags) : static_cast<int>(flags >> 5) - 1;
    if (type == 0x2e) {
        return 7;
    }
    return op;
}

int32_t loot_entry_get_value(uint8_t flags, uint16_t raw) {
    if (flags >= 0x20) {
        int32_t value = (static_cast<int32_t>(flags & 0xf) << 16) | raw;
        if (flags & 0x10) {
            value = -value;
        }
        return value;
    }
    int32_t value = raw & 0x7fff;
    if (static_cast<int16_t>(raw) < 0) {
        value = -value;
    }
    return value;
}

int native_get_loot_filter_rule_entry(void* state) {
    if (!loot_filter || !g_is_num || !g_to_num || !g_push_num || !g_is_num(state, 1)) {
        return 0;
    }
    int32_t rule_index = loot_entry_get_index(g_to_num(state, 1));
    if (rule_index < 0 || rule_index >= static_cast<int>(loot_filter->rule_count)) {
        return 0;
    }
    if (!g_is_num(state, 2)) {
        return 0;
    }
    int32_t entry_index = loot_entry_get_index(g_to_num(state, 2));
    if (entry_index < 0) {
        return 0;
    }
    const uint8_t* rule = loot_filter->rules[rule_index];
    if (entry_index >= static_cast<int>(rule[0x21])) {
        return 0;
    }
    const uint8_t* entry = rule + entry_index * 4;
    uint8_t type = entry[0];
    uint8_t flags = entry[1];
    uint16_t raw = static_cast<uint16_t>(entry[2] | (entry[3] << 8));
    g_push_num(state, static_cast<double>(type));
    g_push_num(state, static_cast<double>(loot_entry_get_op(type, flags)));
    g_push_num(state, static_cast<double>(loot_entry_get_value(flags, raw)));
    return 3;
}
