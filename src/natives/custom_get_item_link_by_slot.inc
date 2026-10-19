using ItemHyperlinkFn = const char* (*)(void* item);

ItemHyperlinkFn item_hyperlink_fn() {
    static ItemHyperlinkFn fn = nullptr;
    static bool ready = false;
    if (!ready) {
        ready = true;
        syn_resolve("_ZN9CGTooltip22GetItemHyperlinkStringEP8CGItem_C", reinterpret_cast<void**>(&fn));
    }
    return fn;
}

// ʕ •ᴥ•ʔ✿ bag, slot. CGTooltip::GetItemHyperlinkString; empty or missing pushes nothing ✿ ʕ •ᴥ•ʔ
int native_get_item_link_by_slot(void* state) {
    if (!g_is_num || !g_to_num || !g_push_str || !g_is_num(state, 1) || !g_is_num(state, 2)) {
        return 0;
    }
    int bag = item_guid_round(g_to_num(state, 1));
    int slot = item_guid_round(g_to_num(state, 2));
    void* item = item_guid_at(bag, slot);
    ItemHyperlinkFn hyperlink = item_hyperlink_fn();
    if (!item || !hyperlink) {
        return 0;
    }
    const char* link = hyperlink(item);
    if (!link || !link[0]) {
        return 0;
    }
    g_push_str(state, link);
    return 1;
}
