int32_t set_extra_int_round(double value) {
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

using set_extra_int_object_fn = void* (*)(uint64_t guid, uint32_t mask, const char* file, int line);
using set_extra_int_feather_fn = void (*)(void* movement);

struct set_extra_int_api_fns {
    set_extra_int_object_fn object = nullptr;
    set_extra_int_feather_fn feather = nullptr;
    bool bound = false;
};

set_extra_int_api_fns& set_extra_int_api() {
    static set_extra_int_api_fns api;
    if (api.bound) {
        return api;
    }
    api.bound = true;
    void* found = nullptr;
    if (syn_resolve("_Z19ClntObjMgrObjectPtr7WOWGUID8TypeMaskPKci", &found)) {
        api.object = reinterpret_cast<set_extra_int_object_fn>(found);
    }
    if (syn_resolve("_ZN15CMovementShared15PostFeatherFallEv", &found)) {
        api.feather = reinterpret_cast<set_extra_int_feather_fn>(found);
    }
    return api;
}

// ʕ •ᴥ•ʔ✿ field 917 refreshes feather fall; movement sits at unit+0xED0 ✿ ʕ •ᴥ•ʔ
void set_extra_int_notify(int32_t field, int32_t guid_lo, int32_t guid_hi) {
    if (field != 0x395) {
        return;
    }
    set_extra_int_api_fns& api = set_extra_int_api();
    if (!api.object || !api.feather) {
        return;
    }
    uint64_t guid = static_cast<uint32_t>(guid_lo)
        | (static_cast<uint64_t>(static_cast<uint32_t>(guid_hi)) << 32);
    void* unit = api.object(guid, 8, "test:2", 0xa0);
    if (!unit) {
        return;
    }
    api.feather(static_cast<char*>(unit) + 0xED0);
}

void set_extra_int_erase(int32_t guid_lo, int32_t guid_hi, int32_t field) {
    auto outer = entity_extra.find(extra_int_key(guid_lo, guid_hi));
    if (outer == entity_extra.end()) {
        return;
    }
    auto slot = outer->second.find(field);
    if (slot == outer->second.end()) {
        return;
    }
    int32_t lo = slot->second.bits_lo;
    int32_t hi = slot->second.bits_hi;
    outer->second.erase(slot);
    if ((lo | hi) != 0) {
        set_extra_int_notify(field, guid_lo, guid_hi);
    }
}

void set_extra_int_store(int32_t guid_lo, int32_t guid_hi, int32_t field, int32_t value, bool create) {
    uint64_t key = extra_int_key(guid_lo, guid_hi);
    auto outer = entity_extra.find(key);
    if (outer == entity_extra.end()) {
        if (!create) {
            return;
        }
        outer = entity_extra.emplace(key, std::map<int32_t, extra_int_cell>{}).first;
    }
    auto slot = outer->second.find(field);
    extra_int_cell cur = slot == outer->second.end() ? extra_int_cell{} : slot->second;
    if (cur.bits_lo == value && cur.bits_hi == 0) {
        return;
    }
    outer->second[field] = extra_int_cell{value, 0};
    set_extra_int_notify(field, guid_lo, guid_hi);
}

int native_set_entity_extra_int(void* state) {
    if (!g_is_num || !g_to_num || !g_push_num || !g_is_num(state, 1) || !g_is_num(state, 2) || !g_is_num(state, 3)) {
        return 0;
    }
    int32_t guid_lo = set_extra_int_round(g_to_num(state, 1));
    int32_t guid_hi = set_extra_int_round(g_to_num(state, 2));
    int32_t field = set_extra_int_round(g_to_num(state, 3));
    if (!g_is_num(state, 4)) {
        set_extra_int_erase(guid_lo, guid_hi, field);
    } else {
        int32_t value = set_extra_int_round(g_to_num(state, 4));
        bool create = g_is_num(state, 5) && set_extra_int_round(g_to_num(state, 5)) > 0;
        set_extra_int_store(guid_lo, guid_hi, field, value, create);
    }
    g_push_num(state, 1.0);
    return 1;
}
