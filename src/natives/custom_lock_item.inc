using LockItemActiveFn = uint64_t (*)();
using LockItemObjectFn = void* (*)(uint64_t guid, uint32_t mask, const char* file, int line);
using LockItemBagFn = void* (*)(void* player);
using LockItemSlotFn = void* (*)(void* bag, int slot);
using LockItemVBagFn = void* (*)(void* item);
using LockItemFn = void (*)(uint64_t guid);

struct lock_item_rec {
    int32_t bag;
    int32_t slot;
    uint32_t low;
    uint32_t high;
};

std::vector<lock_item_rec> lock_item_list;

struct LockItemApi {
    LockItemActiveFn active = nullptr;
    LockItemObjectFn object = nullptr;
    LockItemBagFn bag = nullptr;
    LockItemSlotFn slot = nullptr;
    LockItemFn lock = nullptr;
    bool bound = false;
};

LockItemApi& lock_item_api() {
    static LockItemApi api;
    if (api.bound) {
        return api;
    }
    api.bound = true;
    void* found = nullptr;
    if (syn_resolve("_Z25ClntObjMgrGetActivePlayerv", &found)) {
        api.active = reinterpret_cast<LockItemActiveFn>(found);
    }
    if (syn_resolve("_Z19ClntObjMgrObjectPtr7WOWGUID8TypeMaskPKci", &found)) {
        api.object = reinterpret_cast<LockItemObjectFn>(found);
    }
    if (syn_resolve("_ZNK10CGPlayer_C6GetBagEv", &found)) {
        api.bag = reinterpret_cast<LockItemBagFn>(found);
    }
    if (syn_resolve("_ZNK7CGBag_C14GetItemPointerEi", &found)) {
        api.slot = reinterpret_cast<LockItemSlotFn>(found);
    }
    if (syn_resolve("_ZN8CGGameUI8LockItemE7WOWGUID", &found)) {
        api.lock = reinterpret_cast<LockItemFn>(found);
    }
    return api;
}

int lock_item_round(double n) {
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

void* lock_item_player() {
    LockItemApi& api = lock_item_api();
    if (!api.active || !api.object) {
        return nullptr;
    }
    return api.object(api.active(), 0x10, "test:1", 0xa0);
}

// ʕ •ᴥ•ʔ✿ const GetBag is vtable slot 10 on the container item ✿ ʕ •ᴥ•ʔ
void* lock_item_object_bag(void* item) {
    void* vtable = nullptr;
    memcpy(&vtable, item, sizeof(vtable));
    if (!vtable) {
        return nullptr;
    }
    LockItemVBagFn get_bag = nullptr;
    memcpy(&get_bag, static_cast<char*>(vtable) + 10 * sizeof(void*), sizeof(get_bag));
    if (!get_bag) {
        return nullptr;
    }
    return get_bag(item);
}

void* lock_item_at(int bag_id, int slot) {
    LockItemApi& api = lock_item_api();
    if (!api.bag || !api.slot) {
        return nullptr;
    }
    void* player = lock_item_player();
    if (!player) {
        return nullptr;
    }
    if (static_cast<uint32_t>(slot) > 0x96u) {
        return nullptr;
    }
    void* inv = api.bag(player);
    if (!inv) {
        return nullptr;
    }
    // ʕ •ᴥ•ʔ✿ 255 indexes the player bag; 19-22 and 67-73 are container slots ✿ ʕ •ᴥ•ʔ
    if (bag_id == 0xff) {
        return api.slot(inv, slot);
    }
    bool equipped = bag_id >= 0x13 && bag_id < 0x17;
    bool bank = static_cast<uint32_t>(bag_id - 0x43) <= 6u;
    if (!equipped && !bank) {
        return nullptr;
    }
    void* container = api.slot(inv, bag_id);
    if (!container) {
        return nullptr;
    }
    void* sub = lock_item_object_bag(container);
    if (!sub) {
        return nullptr;
    }
    int32_t num = 0;
    memcpy(&num, sub, sizeof(num));
    if (static_cast<uint32_t>(num) - 1u > 0xfdu) {
        return nullptr;
    }
    if (slot >= num) {
        return nullptr;
    }
    return api.slot(sub, slot);
}

// ʕ •ᴥ•ʔ✿ bag, slot. m_obj at 0x10 holds the guid. A recorded guid pushes nothing. Otherwise CGGameUI::LockItem, which calls CGItem_C::Lock, then guid low and high. ✿ ʕ •ᴥ•ʔ
int native_lock_item(void* state) {
    if (!g_is_num || !g_to_num || !g_push_num || !g_is_num(state, 1) || !g_is_num(state, 2)) {
        return 0;
    }
    int bag = lock_item_round(g_to_num(state, 1));
    int slot = lock_item_round(g_to_num(state, 2));
    void* item = lock_item_at(bag, slot);
    if (!item) {
        return 0;
    }
    void* data = nullptr;
    memcpy(&data, static_cast<char*>(item) + 0x10, sizeof(data));
    if (!data) {
        return 0;
    }
    uint32_t low = 0;
    uint32_t high = 0;
    memcpy(&low, data, sizeof(low));
    memcpy(&high, static_cast<char*>(data) + 4, sizeof(high));
    for (const lock_item_rec& rec : lock_item_list) {
        if (rec.low == low && rec.high == high) {
            return 0;
        }
    }
    LockItemApi& api = lock_item_api();
    if (!api.lock) {
        return 0;
    }
    lock_item_list.push_back({bag, slot, low, high});
    uint64_t guid = static_cast<uint64_t>(low) | (static_cast<uint64_t>(high) << 32);
    api.lock(guid);
    g_push_num(state, static_cast<double>(low));
    g_push_num(state, static_cast<double>(high));
    return 2;
}
