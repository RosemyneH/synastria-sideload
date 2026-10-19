int32_t loot_rule_set_round(double value) {
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

bool loot_rule_set_index(double value, int* index) {
    constexpr double k_min = -2147483648.0;
    constexpr double k_max = 2147483647.0;
    if (!(value > k_min) || !(value < k_max)) {
        return false;
    }
    double rounded = __builtin_round(value);
    if (!(rounded > k_min) || !(rounded < k_max)) {
        return false;
    }
    int slot = static_cast<int>(rounded) - 1;
    if (static_cast<uint32_t>(slot) > 31u) {
        return false;
    }
    *index = slot;
    return true;
}

// ʕ •ᴥ•ʔ✿ rule+0x24 is the x86 MSVC string. Size is +0x34, capacity +0x38. Below 16 the bytes are inline; a host pointer is stored when the name outgrows that. A capacity under 15 has never been constructed. ✿ ʕ •ᴥ•ʔ
void loot_rule_set_name(uint8_t* rule, const char* src) {
    if (!src) {
        return;
    }
    size_t len = 0;
    while (src[len] != 0) {
        ++len;
    }
    uint8_t* text = rule + 0x24;
    uint32_t old_cap = 0;
    std::memcpy(&old_cap, text + 0x14, sizeof(old_cap));
    uint32_t cap = old_cap < 15u ? 15u : old_cap;
    if (len <= cap) {
        char* dest = reinterpret_cast<char*>(text);
        if (cap >= 16u) {
            void* ptr = nullptr;
            std::memcpy(&ptr, text, sizeof(ptr));
            dest = static_cast<char*>(ptr);
        }
        if (len != 0) {
            std::memcpy(dest, src, len);
        }
        dest[len] = 0;
        uint32_t stored = static_cast<uint32_t>(len);
        std::memcpy(text + 0x10, &stored, sizeof(stored));
        if (old_cap < 15u) {
            std::memcpy(text + 0x14, &cap, sizeof(cap));
        }
        return;
    }
    if (len > 0x7fffffffu) {
        return;
    }
    uint32_t grown = static_cast<uint32_t>(len) | 0x0fu;
    if (grown > 0x7fffffffu) {
        grown = 0x7fffffffu;
    } else {
        uint32_t half = cap >> 1;
        if (cap > 0x7fffffffu - half) {
            grown = 0x7fffffffu;
        } else {
            uint32_t sum = cap + half;
            if (grown < sum) {
                grown = sum;
            }
        }
    }
    char* buf = static_cast<char*>(std::malloc(static_cast<size_t>(grown) + 1u));
    if (!buf) {
        return;
    }
    std::memcpy(buf, src, len);
    buf[len] = 0;
    if (old_cap >= 16u) {
        void* old = nullptr;
        std::memcpy(&old, text, sizeof(old));
        std::free(old);
    }
    void* stored_ptr = buf;
    std::memcpy(text, &stored_ptr, sizeof(stored_ptr));
    uint32_t stored = static_cast<uint32_t>(len);
    std::memcpy(text + 0x10, &stored, sizeof(stored));
    std::memcpy(text + 0x14, &grown, sizeof(grown));
}

int native_set_loot_filter_rule(void* state) {
    if (!loot_filter || !g_is_num || !g_to_num || !g_is_num(state, 1)) {
        return 0;
    }
    int index = 0;
    if (!loot_rule_set_index(g_to_num(state, 1), &index)) {
        return 0;
    }
    uint8_t* rule = loot_filter->rules[index];
    for (int slot = 0; slot < 4; ++slot) {
        uint8_t byte = 0;
        if (g_is_num(state, slot + 2)) {
            byte = static_cast<uint8_t>(loot_rule_set_round(g_to_num(state, slot + 2)));
        }
        rule[0x20 + slot] = byte;
    }
    if (g_is_str && g_to_str && g_is_str(state, 6)) {
        loot_rule_set_name(rule, g_to_str(state, 6, nullptr));
    }
    if (rule[0x21] > 8) {
        rule[0x21] = 8;
    }
    return 0;
}
