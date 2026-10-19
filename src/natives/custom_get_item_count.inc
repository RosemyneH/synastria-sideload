int item_count_round(double n) {
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

using ActivePlayerFn = uint64_t (*)();
using ObjectPtrFn = void* (*)(uint64_t guid, uint32_t mask, const char* file, int line);
using PlayerBagFn = void* (*)(void* player);
using SlotItemFn = void* (*)(void* bag, int slot);
using ObjectBagFn = void* (*)(void* object);

struct ItemCountApi {
    ActivePlayerFn active = nullptr;
    ObjectPtrFn object = nullptr;
    PlayerBagFn bag = nullptr;
    SlotItemFn slot = nullptr;
    bool bound = false;
};

ItemCountApi& item_count_api() {
    static ItemCountApi api;
    if (api.bound) {
        return api;
    }
    api.bound = true;
    void* found = nullptr;
    if (syn_resolve("_Z25ClntObjMgrGetActivePlayerv", &found)) {
        api.active = reinterpret_cast<ActivePlayerFn>(found);
    }
    if (syn_resolve("_Z19ClntObjMgrObjectPtr7WOWGUID8TypeMaskPKci", &found)) {
        api.object = reinterpret_cast<ObjectPtrFn>(found);
    }
    if (syn_resolve("_ZNK10CGPlayer_C6GetBagEv", &found)) {
        api.bag = reinterpret_cast<PlayerBagFn>(found);
    }
    if (syn_resolve("_ZNK7CGBag_C14GetItemPointerEi", &found)) {
        api.slot = reinterpret_cast<SlotItemFn>(found);
    }
    return api;
}

void* item_count_player() {
    ItemCountApi& api = item_count_api();
    if (!api.active || !api.object) {
        return nullptr;
    }
    return api.object(api.active(), 0x10, "test:1", 0xa0);
}

// ʕ •ᴥ•ʔ✿ const GetBag is vtable slot 10 on the container item ✿ ʕ •ᴥ•ʔ
void* item_count_object_bag(void* item) {
    void* vtable = nullptr;
    memcpy(&vtable, item, sizeof(vtable));
    if (!vtable) {
        return nullptr;
    }
    ObjectBagFn get_bag = nullptr;
    memcpy(&get_bag, static_cast<char*>(vtable) + 10 * sizeof(void*), sizeof(get_bag));
    if (!get_bag) {
        return nullptr;
    }
    return get_bag(item);
}

void* item_count_at(int bag_id, int slot) {
    ItemCountApi& api = item_count_api();
    if (!api.bag || !api.slot) {
        return nullptr;
    }
    void* player = item_count_player();
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
    void* sub = item_count_object_bag(container);
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

// ʕ •ᴥ•ʔ✿ CGItem_C::m_item at 0x168, CGItemData::m_itemFieldStackCount at 0x20 ✿ ʕ •ᴥ•ʔ
int32_t item_stack_count(void* item) {
    void* data = nullptr;
    memcpy(&data, static_cast<char*>(item) + 0x168, sizeof(data));
    if (!data) {
        return 0;
    }
    int32_t count = 0;
    memcpy(&count, static_cast<char*>(data) + 0x20, sizeof(count));
    return count;
}

int native_get_item_count(void* state) {
    if (!g_is_num || !g_to_num || !g_is_num(state, 1) || !g_is_num(state, 2)) {
        return 0;
    }
    int bag_id = item_count_round(g_to_num(state, 1));
    int slot = item_count_round(g_to_num(state, 2));
    void* item = item_count_at(bag_id, slot);
    if (!item) {
        return 0;
    }
    g_push_num(state, item_stack_count(item));
    return 1;
}
