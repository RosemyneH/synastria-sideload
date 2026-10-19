// ʕ •ᴥ•ʔ✿ zero-terminated enchant ids; a query compares 25 then each later word, and list slots 1 and 2 are the ids 0 and 1 ✿ ʕ •ᴥ•ʔ
const uint16_t tmog_ench_ids[] = {
    25, 805, 1896, 803, 7, 26, 13, 1890, 249, 3, 2, 943, 6, 283, 1606, 3846,
    1900, 3093, 2668, 1023, 1103, 1894, 3855, 2343, 2673, 2667, 2672, 2671,
    2669, 2674, 1899, 3870, 3225, 3273, 3854, 3364, 3789, 3869, 0,
};

bool tmog_ench_int(double n, int32_t* out, int32_t low, int32_t high) {
    constexpr double k_min = -2147483648.0;
    constexpr double k_max = 2147483647.0;
    if (!(n > k_min)) {
        *out = low;
        return false;
    }
    if (!(n < k_max)) {
        *out = high;
        return false;
    }
    double rounded = __builtin_round(n);
    if (!(rounded > k_min)) {
        *out = low;
        return false;
    }
    if (!(rounded < k_max)) {
        *out = high;
        return false;
    }
    *out = static_cast<int32_t>(rounded);
    return true;
}

int tmog_ench_query(void* state) {
    int32_t sel = -1;
    if (g_is_num(state, 2)) {
        tmog_ench_int(g_to_num(state, 2), &sel, static_cast<int32_t>(0x80000000u), 0x7fffffff);
    }
    int found = -1;
    int prev = 0x19;
    int index = 0;
    uint16_t word = 0;
    do {
        prev &= 0xffff;
        int slot = index;
        if (prev != sel) {
            slot = found;
        }
        word = tmog_ench_ids[index + 1];
        index++;
        found = slot;
        prev = word;
    } while (word != 0);
    double picked = 0.0;
    if (static_cast<uint32_t>(sel) <= 1u) {
        picked = static_cast<double>(sel + 1);
    } else if (found >= 0) {
        picked = static_cast<double>(found + 3);
    }
    g_push_num(state, static_cast<double>(index + 2));
    g_push_num(state, picked);
    return 2;
}

int tmog_ench_at(void* state) {
    int32_t slot = 0;
    if (!g_is_num(state, 2)) {
        slot = static_cast<int32_t>(static_cast<uint32_t>(-1) - 2u);
    } else {
        int32_t value = 0;
        if (!tmog_ench_int(g_to_num(state, 2), &value, 0x7fffffff, 0x7ffffffe)) {
            slot = static_cast<int32_t>(static_cast<uint32_t>(value) - 2u);
        } else {
            int32_t dec = static_cast<int32_t>(static_cast<uint32_t>(value) - 1u);
            if (static_cast<uint32_t>(dec) <= 1u) {
                g_push_num(state, static_cast<double>(dec));
                return 1;
            }
            slot = static_cast<int32_t>(static_cast<uint32_t>(dec) - 2u);
        }
    }
    int span = 0;
    do {
        span++;
    } while (tmog_ench_ids[span] != 0);
    if (slot < 0 || slot >= span) {
        return 0;
    }
    g_push_num(state, static_cast<double>(tmog_ench_ids[slot]));
    return 1;
}

int native_get_transmog_enchant(void* state) {
    if (!g_is_num || !g_to_num || !g_push_num || !g_is_num(state, 1)) {
        return 0;
    }
    int32_t mode = 0;
    if (!tmog_ench_int(g_to_num(state, 1), &mode, 0, 0)) {
        return 0;
    }
    if (mode == 0) {
        return tmog_ench_query(state);
    }
    if (mode == 1) {
        return tmog_ench_at(state);
    }
    return 0;
}
