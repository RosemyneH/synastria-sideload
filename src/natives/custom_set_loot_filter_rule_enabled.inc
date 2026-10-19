int32_t loot_rule_en_int(double value) {
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

using loot_rule_en_ctor_fn = void (*)(void*);
using loot_rule_en_dtor_fn = void (*)(void*);
using loot_rule_en_put_u8_fn = void (*)(void*, uint8_t);
using loot_rule_en_put_u32_fn = void (*)(void*, uint32_t);
using loot_rule_en_finalize_fn = void (*)(void*);
using loot_rule_en_send_fn = void (*)(void*);

struct loot_rule_en_api {
    loot_rule_en_ctor_fn ctor = nullptr;
    loot_rule_en_dtor_fn dtor = nullptr;
    loot_rule_en_put_u8_fn put_u8 = nullptr;
    loot_rule_en_put_u32_fn put_u32 = nullptr;
    loot_rule_en_finalize_fn finalize = nullptr;
    loot_rule_en_send_fn send = nullptr;
    bool bound = false;
};

loot_rule_en_api& loot_rule_en_bind() {
    static loot_rule_en_api api;
    if (api.bound) {
        return api;
    }
    api.bound = true;
    void* found = nullptr;
    if (syn_resolve("_ZN10CDataStoreC1Ev", &found)) {
        api.ctor = reinterpret_cast<loot_rule_en_ctor_fn>(found);
    }
    if (syn_resolve("_ZN10CDataStoreD1Ev", &found)) {
        api.dtor = reinterpret_cast<loot_rule_en_dtor_fn>(found);
    }
    if (syn_resolve("_ZN10CDataStore3PutEh", &found)) {
        api.put_u8 = reinterpret_cast<loot_rule_en_put_u8_fn>(found);
    }
    if (syn_resolve("_ZN10CDataStore3PutEj", &found)) {
        api.put_u32 = reinterpret_cast<loot_rule_en_put_u32_fn>(found);
    }
    if (syn_resolve("_ZN10CDataStore8FinalizeEv", &found)) {
        api.finalize = reinterpret_cast<loot_rule_en_finalize_fn>(found);
    }
    if (syn_resolve("_ZN14ClientServices5Send2EP10CDataStore", &found)) {
        api.send = reinterpret_cast<loot_rule_en_send_fn>(found);
    }
    return api;
}

// ʕ •ᴥ•ʔ✿ opcode 0x4b8, then bytes 0x0f, 0x03, zero-based rule, 1 if enabling ✿ ʕ •ᴥ•ʔ
void loot_rule_en_notify(uint8_t index, uint8_t enabled) {
    loot_rule_en_api& api = loot_rule_en_bind();
    if (!api.ctor || !api.dtor || !api.put_u8 || !api.put_u32 || !api.finalize || !api.send) {
        return;
    }
    alignas(16) unsigned char raw[64];
    std::memset(raw, 0, sizeof(raw));
    api.ctor(raw);
    api.put_u32(raw, 0x4b8u);
    api.put_u8(raw, 0x0f);
    api.put_u8(raw, 0x03);
    api.put_u8(raw, index);
    api.put_u8(raw, enabled);
    api.finalize(raw);
    api.send(raw);
    api.dtor(raw);
}

// ʕ •ᴥ•ʔ✿ rule+0x20 bit 0x80 means disabled. Same state returns without a packet. ✿ ʕ •ᴥ•ʔ
int native_set_loot_filter_rule_enabled(void* state) {
    if (!loot_filter || !g_is_num || !g_to_num || !g_is_num(state, 1) || !g_is_num(state, 2)) {
        return 0;
    }
    int32_t slot = loot_rule_en_int(g_to_num(state, 1));
    int32_t flag = loot_rule_en_int(g_to_num(state, 2));
    int32_t index = static_cast<int32_t>(static_cast<uint32_t>(slot) - 1u);
    if (index < 0 || index >= 32) {
        return 0;
    }
    uint8_t* byte = &loot_filter->rules[index][0x20];
    if (flag != 0) {
        if ((*byte & 0x80) == 0) {
            return 0;
        }
        *byte = static_cast<uint8_t>(*byte & 0x7f);
    } else {
        if ((*byte & 0x80) != 0) {
            return 0;
        }
        *byte = static_cast<uint8_t>(*byte | 0x80);
    }
    loot_rule_en_notify(static_cast<uint8_t>(index), flag != 0 ? 1 : 0);
    return 0;
}
