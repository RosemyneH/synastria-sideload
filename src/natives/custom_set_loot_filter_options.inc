using loot_opt_set_ctor_fn = void (*)(void*);
using loot_opt_set_dtor_fn = void (*)(void*);
using loot_opt_set_put_u8_fn = void (*)(void*, uint8_t);
using loot_opt_set_put_u16_fn = void (*)(void*, uint16_t);
using loot_opt_set_put_u32_fn = void (*)(void*, uint32_t);
using loot_opt_set_fin_fn = void (*)(void*);
using loot_opt_set_send_fn = void (*)(void*);

struct loot_opt_set_api {
    loot_opt_set_ctor_fn ctor = nullptr;
    loot_opt_set_dtor_fn dtor = nullptr;
    loot_opt_set_put_u8_fn put_u8 = nullptr;
    loot_opt_set_put_u16_fn put_u16 = nullptr;
    loot_opt_set_put_u32_fn put_u32 = nullptr;
    loot_opt_set_fin_fn fin = nullptr;
    loot_opt_set_send_fn send = nullptr;
    bool bound = false;
};

loot_opt_set_api& loot_opt_set_bind() {
    static loot_opt_set_api api;
    if (api.bound) {
        return api;
    }
    api.bound = true;
    void* found = nullptr;
    if (syn_resolve("_ZN10CDataStoreC1Ev", &found)) {
        api.ctor = reinterpret_cast<loot_opt_set_ctor_fn>(found);
    }
    if (syn_resolve("_ZN10CDataStoreD1Ev", &found)) {
        api.dtor = reinterpret_cast<loot_opt_set_dtor_fn>(found);
    }
    if (syn_resolve("_ZN10CDataStore3PutEh", &found)) {
        api.put_u8 = reinterpret_cast<loot_opt_set_put_u8_fn>(found);
    }
    if (syn_resolve("_ZN10CDataStore3PutEt", &found)) {
        api.put_u16 = reinterpret_cast<loot_opt_set_put_u16_fn>(found);
    }
    if (syn_resolve("_ZN10CDataStore3PutEj", &found)) {
        api.put_u32 = reinterpret_cast<loot_opt_set_put_u32_fn>(found);
    }
    if (syn_resolve("_ZN10CDataStore8FinalizeEv", &found)) {
        api.fin = reinterpret_cast<loot_opt_set_fin_fn>(found);
    }
    if (syn_resolve("_ZN14ClientServices5Send2EP10CDataStore", &found)) {
        api.send = reinterpret_cast<loot_opt_set_send_fn>(found);
    }
    return api;
}

int32_t loot_opt_set_round(double value) {
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

void loot_opt_set_send(uint16_t options) {
    loot_opt_set_api& api = loot_opt_set_bind();
    if (!api.ctor || !api.dtor || !api.put_u8 || !api.put_u16 || !api.put_u32 || !api.fin || !api.send) {
        return;
    }
    alignas(16) unsigned char raw[64];
    api.ctor(raw);
    api.put_u32(raw, 0x4B8u);
    api.put_u8(raw, 0x0f);
    api.put_u8(raw, 0x04);
    api.put_u16(raw, options);
    api.fin(raw);
    api.send(raw);
    api.dtor(raw);
}

// ʕ •ᴥ•ʔ✿ one number. Stores the low 16 bits in options and, when they change, sends opcode 0x4B8 then bytes 0x0F 0x04 and that word. Returns nothing. ✿ ʕ •ᴥ•ʔ
int native_set_loot_filter_options(void* state) {
    if (!loot_filter || !g_is_num || !g_to_num || !g_is_num(state, 1)) {
        return 0;
    }
    uint16_t options = static_cast<uint16_t>(loot_opt_set_round(g_to_num(state, 1)));
    if (loot_filter->options == options) {
        return 0;
    }
    loot_filter->options = options;
    loot_opt_set_send(options);
    return 0;
}
