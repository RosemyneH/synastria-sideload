using UnlockItemFn = void (*)(uint64_t guid);

UnlockItemFn unlock_item_fn() {
    static UnlockItemFn fn = nullptr;
    static bool bound = false;
    if (bound) {
        return fn;
    }
    bound = true;
    void* found = nullptr;
    if (syn_resolve("_ZN8CGGameUI10UnlockItemE7WOWGUID", &found)) {
        fn = reinterpret_cast<UnlockItemFn>(found);
    }
    return fn;
}

bool unlock_item_same(void* item, uint32_t low, uint32_t high) {
    if (!item) {
        return false;
    }
    void* data = nullptr;
    memcpy(&data, static_cast<char*>(item) + 0x10, sizeof(data));
    if (!data) {
        return false;
    }
    uint32_t cur_low = 0;
    uint32_t cur_high = 0;
    memcpy(&cur_low, data, sizeof(cur_low));
    memcpy(&cur_high, static_cast<char*>(data) + 4, sizeof(cur_high));
    return cur_low == low && cur_high == high;
}

// ʕ •ᴥ•ʔ✿ Slot guid match, else ClntObjMgrObjectPtr with TYPEMASK_ITEM. Either path calls CGGameUI::UnlockItem. ✿ ʕ •ᴥ•ʔ
void unlock_item_apply(const lock_item_rec& rec) {
    uint64_t guid = static_cast<uint64_t>(rec.low) | (static_cast<uint64_t>(rec.high) << 32);
    bool live = unlock_item_same(lock_item_at(rec.bag, rec.slot), rec.low, rec.high);
    if (!live) {
        LockItemApi& api = lock_item_api();
        if (!api.object || !api.object(guid, 2, "test:2", 0xa0)) {
            return;
        }
    }
    UnlockItemFn fn = unlock_item_fn();
    if (fn) {
        fn(guid);
    }
}

// ʕ •ᴥ•ʔ✿ guid low, guid high. Drops the first matching lock record and pushes 1. ✿ ʕ •ᴥ•ʔ
int native_unlock_item(void* state) {
    if (!g_is_num || !g_to_num || !g_push_num || !g_is_num(state, 1) || !g_is_num(state, 2)) {
        return 0;
    }
    uint32_t low = static_cast<uint32_t>(lock_item_round(g_to_num(state, 1)));
    uint32_t high = static_cast<uint32_t>(lock_item_round(g_to_num(state, 2)));
    for (auto it = lock_item_list.begin(); it != lock_item_list.end(); ++it) {
        if (it->low != low || it->high != high) {
            continue;
        }
        lock_item_rec rec = *it;
        lock_item_list.erase(it);
        unlock_item_apply(rec);
        g_push_num(state, 1.0);
        return 1;
    }
    return 0;
}
