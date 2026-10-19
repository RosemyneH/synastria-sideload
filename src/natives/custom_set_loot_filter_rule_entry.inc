int32_t loot_entry_set_round(double value) {
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

bool loot_entry_set_index(void* state, int arg, uint32_t limit, int* out) {
    if (!g_is_num(state, arg)) {
        return false;
    }
    int32_t rounded = loot_entry_set_round(g_to_num(state, arg));
    if (rounded == static_cast<int32_t>(0x80000000u) || rounded == 0x7fffffff) {
        return false;
    }
    int index = rounded - 1;
    if (static_cast<uint32_t>(index) > limit) {
        return false;
    }
    *out = index;
    return true;
}

int32_t loot_entry_set_arg(void* state, int arg) {
    if (!g_is_num(state, arg)) {
        return 0;
    }
    return loot_entry_set_round(g_to_num(state, arg));
}

int32_t loot_entry_set_decode(uint8_t op, uint16_t word) {
    if (op & 0xe0) {
        uint32_t magnitude = (static_cast<uint32_t>(op & 0xf) << 16) | word;
        if (op & 0x10) {
            return -static_cast<int32_t>(magnitude);
        }
        return static_cast<int32_t>(magnitude);
    }
    uint32_t magnitude = word & 0x7fff;
    if (word & 0x8000) {
        return -static_cast<int32_t>(magnitude);
    }
    return static_cast<int32_t>(magnitude);
}

// ʕ •ᴥ•ʔ✿ dword is type, op, value. Types 45 and 46 clear op. Past ±32767 the op byte carries the scale. Type byte 0xff pushes the value and does not store. ✿ ʕ •ᴥ•ʔ
bool loot_entry_set_pack(int32_t type, int32_t op, int32_t value, uint32_t* bits, double* shown) {
    uint8_t type_b = static_cast<uint8_t>(type);
    if (type == 0x2d || type == 0x2e) {
        op = 0;
    }
    uint8_t op_b = static_cast<uint8_t>(op);
    uint16_t word = 0;
    if (op_b & 0xe0) {
        *shown = static_cast<double>(loot_entry_set_decode(0, 1));
        return false;
    }
    if (value >= -32767 && value <= 32767) {
        if (value < 0) {
            word = static_cast<uint16_t>((-value) | 0x8000);
        } else {
            word = static_cast<uint16_t>(value);
        }
    } else if (value < -1048575 || value > 1048575) {
        *shown = static_cast<double>(loot_entry_set_decode(op_b, 2));
        return false;
    } else if (op_b >= 7) {
        *shown = static_cast<double>(loot_entry_set_decode(op_b, 3));
        return false;
    } else {
        op_b = static_cast<uint8_t>((op_b + 1) << 5);
        int32_t magnitude = value;
        if (magnitude < 0) {
            magnitude = -magnitude;
            op_b = static_cast<uint8_t>(op_b | 0x10);
        }
        word = static_cast<uint16_t>(magnitude);
        op_b = static_cast<uint8_t>(op_b | ((magnitude >> 16) & 0xf));
    }
    if (type_b == 0xff) {
        *shown = static_cast<double>(loot_entry_set_decode(op_b, word));
        return false;
    }
    *bits = type_b | (static_cast<uint32_t>(op_b) << 8) | (static_cast<uint32_t>(word) << 16);
    return true;
}

int native_set_loot_filter_rule_entry(void* state) {
    if (!loot_filter || !g_is_num || !g_to_num) {
        return 0;
    }
    int rule = 0;
    int entry = 0;
    if (!loot_entry_set_index(state, 1, 0x1f, &rule) || !loot_entry_set_index(state, 2, 7, &entry)) {
        return 0;
    }
    uint32_t bits = 0;
    double shown = 0;
    if (!loot_entry_set_pack(loot_entry_set_arg(state, 3), loot_entry_set_arg(state, 4), loot_entry_set_arg(state, 5), &bits, &shown)) {
        if (!g_push_num) {
            return 0;
        }
        g_push_num(state, shown);
        return 1;
    }
    *reinterpret_cast<uint32_t*>(&loot_filter->rules[rule][entry * 4]) = bits;
    return 0;
}
