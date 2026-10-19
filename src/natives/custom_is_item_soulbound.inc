using ItemBoundFn = bool (*)(void* item);

ItemBoundFn item_bound_fn() {
    static ItemBoundFn fn = nullptr;
    static bool ready = false;
    if (!ready) {
        ready = true;
        syn_resolve("_ZN8CGItem_C7IsBoundEv", reinterpret_cast<void**>(&fn));
    }
    return fn;
}

// ʕ •ᴥ•ʔ✿ bag, slot. CGItem_C::IsBound reads CGItemData::m_itemFieldFlags bit 0, else a soulbound enchant. Pushes 1 or nothing. ✿ ʕ •ᴥ•ʔ
int native_is_item_soulbound(void* state) {
    if (!g_is_num || !g_to_num || !g_push_num || !g_is_num(state, 1) || !g_is_num(state, 2)) {
        return 0;
    }
    int bag = item_count_round(g_to_num(state, 1));
    int slot = item_count_round(g_to_num(state, 2));
    void* item = item_count_at(bag, slot);
    ItemBoundFn bound = item_bound_fn();
    if (!item || !bound || !bound(item)) {
        return 0;
    }
    g_push_num(state, 1.0);
    return 1;
}
