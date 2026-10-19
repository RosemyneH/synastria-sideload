// ʕ •ᴥ•ʔ✿ entity_extra is the one shared store. Outer key is the guid packed low dword then high dword: (uint32)low | ((uint64)(uint32)high << 32). Inner key is the field id. Each cell is 8 bytes, bits_lo then bits_hi. GetInt pushes bits_lo as a signed int. GetFloat pushes both dwords as one IEEE double. A miss uses the caller's default: GetInt passes lua arg 4 (else 0) as bits_lo and 0 as bits_hi. ✿ ʕ •ᴥ•ʔ
struct extra_int_cell {
    int32_t bits_lo = 0;
    int32_t bits_hi = 0;
};

std::map<uint64_t, std::map<int32_t, extra_int_cell>> entity_extra;

int extra_int_round(double n) {
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

uint64_t extra_int_key(int32_t low, int32_t high) {
    return static_cast<uint32_t>(low)
        | (static_cast<uint64_t>(static_cast<uint32_t>(high)) << 32);
}

int32_t extra_int_load(int32_t low, int32_t high, int32_t field, int32_t fallback) {
    auto ent = entity_extra.find(extra_int_key(low, high));
    if (ent == entity_extra.end()) {
        return fallback;
    }
    auto cell = ent->second.find(field);
    if (cell == ent->second.end()) {
        return fallback;
    }
    return cell->second.bits_lo;
}

int native_get_entity_extra_int(void* state) {
    if (!g_is_num || !g_to_num || !g_push_num || !g_is_num(state, 1) || !g_is_num(state, 2) || !g_is_num(state, 3)) {
        return 0;
    }
    int32_t low = extra_int_round(g_to_num(state, 1));
    int32_t high = extra_int_round(g_to_num(state, 2));
    int32_t field = extra_int_round(g_to_num(state, 3));
    int32_t fallback = 0;
    if (g_is_num(state, 4)) {
        fallback = extra_int_round(g_to_num(state, 4));
    }
    g_push_num(state, static_cast<double>(extra_int_load(low, high, field, fallback)));
    return 1;
}
