struct MerchantSlot {
    int32_t slot;
    int32_t item_id;
    int32_t pad[6];
};

struct MerchantView {
    MerchantSlot* items = nullptr;
    int32_t* count = nullptr;
    uint64_t* merchant = nullptr;
    bool bound = false;
};

MerchantView& merchant_view() {
    static MerchantView view;
    if (view.bound) {
        return view;
    }
    view.bound = true;
    void* found = nullptr;
    if (syn_resolve("_ZN14CGMerchantInfo7m_itemsE", &found)) {
        view.items = static_cast<MerchantSlot*>(found);
    }
    found = nullptr;
    if (syn_resolve("_ZN14CGMerchantInfo18m_numMerchantItemsE", &found)) {
        view.count = static_cast<int32_t*>(found);
    }
    found = nullptr;
    if (syn_resolve("_ZN14CGMerchantInfo10m_merchantE", &found)) {
        view.merchant = static_cast<uint64_t*>(found);
    }
    return view;
}

// ʕ •ᴥ•ʔ✿ same bounds as the in-DLL lua number helper: round, then clamp to int32 ✿ ʕ •ᴥ•ʔ
int merchant_lua_int(double n) {
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

uint32_t merchant_quality_color(uint8_t quality) {
    static const uint32_t colors[] = {
        0xff9d9d9du, 0xffffffffu, 0xff1eff00u, 0xff0070ddu,
        0xffa335eeu, 0xffff8000u, 0xffe6cc80u, 0xffe6cc80u,
    };
    if (quality > 7) {
        return 0xffe6cc80u;
    }
    return colors[quality];
}

// ʕ •ᴥ•ʔ✿ 1-based slot; item id, then |c color |Hitem:id:0:0:0:0:0:0:0:80|h[name]|h|r ✿ ʕ •ᴥ•ʔ
int native_get_merchant_item(void* state) {
    if (!g_is_num || !g_to_num || !g_push_num || !g_push_str || !g_is_num(state, 1)) {
        return 0;
    }
    int slot = merchant_lua_int(g_to_num(state, 1));
    if (slot <= 0) {
        return 0;
    }
    int index = slot - 1;
    MerchantView& view = merchant_view();
    if (!view.items || !view.count || !view.merchant || !*view.merchant) {
        return 0;
    }
    if (index >= *view.count) {
        return 0;
    }
    int32_t item_id = view.items[index].item_id;
    if (item_id <= 0 || static_cast<uint32_t>(item_id) > 0xffffu) {
        return 0;
    }
    const ItemRow* row = item_row(item_id);
    if (!row) {
        return 0;
    }
    char link[512];
    snprintf(link, sizeof(link), "|c%x|Hitem:%d:0:0:0:0:0:0:0:80|h[%s]|h|r",
             merchant_quality_color(row->quality), item_id, row->name.c_str());
    g_push_num(state, item_id);
    g_push_str(state, link);
    return 2;
}
