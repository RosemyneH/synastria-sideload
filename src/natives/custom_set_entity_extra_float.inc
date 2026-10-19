using set_extra_float_object_fn = void* (*)(uint64_t guid, uint32_t mask, const char* file, int line);
using set_extra_float_feather_fn = void (*)(void* movement);

struct set_extra_float_api_fns {
    set_extra_float_object_fn object = nullptr;
    set_extra_float_feather_fn feather = nullptr;
    bool bound = false;
};

set_extra_float_api_fns& set_extra_float_api() {
    static set_extra_float_api_fns api;
    if (api.bound) {
        return api;
    }
    api.bound = true;
    void* found = nullptr;
    if (syn_resolve("_Z19ClntObjMgrObjectPtr7WOWGUID8TypeMaskPKci", &found)) {
        api.object = reinterpret_cast<set_extra_float_object_fn>(found);
    }
    if (syn_resolve("_ZN15CMovementShared15PostFeatherFallEv", &found)) {
        api.feather = reinterpret_cast<set_extra_float_feather_fn>(found);
    }
    return api;
}

// ʕ •ᴥ•ʔ✿ field 917 refreshes feather fall; movement object is at unit+0xED0 ✿ ʕ •ᴥ•ʔ
void set_extra_float_notify(int32_t field, int32_t guid_lo, int32_t guid_hi) {
    if (field != 0x395) {
        return;
    }
    set_extra_float_api_fns& api = set_extra_float_api();
    if (!api.object || !api.feather) {
        return;
    }
    void* unit = api.object(extra_int_key(guid_lo, guid_hi), 8, "test:2", 0xa0);
    if (!unit) {
        return;
    }
    api.feather(static_cast<char*>(unit) + 0xED0);
}

void set_extra_float_split(double value, int32_t& lo, int32_t& hi) {
    uint64_t bits = 0;
    memcpy(&bits, &value, sizeof(bits));
    lo = static_cast<int32_t>(static_cast<uint32_t>(bits));
    hi = static_cast<int32_t>(static_cast<uint32_t>(bits >> 32));
}

void set_extra_float_erase(int32_t guid_lo, int32_t guid_hi, int32_t field) {
    auto ent = entity_extra.find(extra_int_key(guid_lo, guid_hi));
    if (ent == entity_extra.end()) {
        return;
    }
    auto cell = ent->second.find(field);
    if (cell == ent->second.end()) {
        return;
    }
    int32_t lo = cell->second.bits_lo;
    int32_t hi = cell->second.bits_hi;
    ent->second.erase(cell);
    if ((lo | hi) != 0) {
        set_extra_float_notify(field, guid_lo, guid_hi);
    }
}

// ʕ •ᴥ•ʔ✿ a missing field compares as +0, so an equal value skips the write. create still inserts an empty entity ✿ ʕ •ᴥ•ʔ
void set_extra_float_store(int32_t guid_lo, int32_t guid_hi, int32_t field, double value, bool create) {
    uint64_t key = extra_int_key(guid_lo, guid_hi);
    auto ent = entity_extra.find(key);
    if (ent == entity_extra.end()) {
        if (!create) {
            return;
        }
        ent = entity_extra.emplace(key, std::map<int32_t, extra_int_cell>{}).first;
    }
    int32_t lo = 0;
    int32_t hi = 0;
    set_extra_float_split(value, lo, hi);
    auto cell = ent->second.find(field);
    int32_t cur_lo = 0;
    int32_t cur_hi = 0;
    if (cell != ent->second.end()) {
        cur_lo = cell->second.bits_lo;
        cur_hi = cell->second.bits_hi;
    }
    if (cur_lo == lo && cur_hi == hi) {
        return;
    }
    ent->second[field] = extra_int_cell{lo, hi};
    set_extra_float_notify(field, guid_lo, guid_hi);
}

int native_set_entity_extra_float(void* state) {
    if (!g_is_num || !g_to_num || !g_push_num || !g_is_num(state, 1) || !g_is_num(state, 2) || !g_is_num(state, 3)) {
        return 0;
    }
    int32_t guid_lo = extra_int_round(g_to_num(state, 1));
    int32_t guid_hi = extra_int_round(g_to_num(state, 2));
    int32_t field = extra_int_round(g_to_num(state, 3));
    if (!g_is_num(state, 4)) {
        set_extra_float_erase(guid_lo, guid_hi, field);
    } else {
        double value = g_to_num(state, 4);
        bool create = g_is_num(state, 5) && extra_int_round(g_to_num(state, 5)) > 0;
        set_extra_float_store(guid_lo, guid_hi, field, value, create);
    }
    g_push_num(state, 1.0);
    return 1;
}
