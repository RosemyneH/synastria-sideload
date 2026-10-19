int item_guid_round(double n) {
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

using ItemGuidActiveFn = uint64_t (*)();
using ItemGuidObjectFn = void* (*)(uint64_t guid, uint32_t mask, const char* file, int line);
using ItemGuidBagFn = void* (*)(void* player);
using ItemGuidSlotFn = void* (*)(void* bag, int slot);
using ItemGuidObjectBagFn = void* (*)(void* object);

struct ItemGuidApi {
    ItemGuidActiveFn active = nullptr;
    ItemGuidObjectFn object = nullptr;
    ItemGuidBagFn bag = nullptr;
    ItemGuidSlotFn slot = nullptr;
    bool bound = false;
};

ItemGuidApi& item_guid_api() {
    static ItemGuidApi api;
    if (api.bound) {
        return api;
    }
    api.bound = true;
    void* found = nullptr;
    if (syn_resolve("_Z25ClntObjMgrGetActivePlayerv", &found)) {
        api.active = reinterpret_cast<ItemGuidActiveFn>(found);
    }
    if (syn_resolve("_Z19ClntObjMgrObjectPtr7WOWGUID8TypeMaskPKci", &found)) {
        api.object = reinterpret_cast<ItemGuidObjectFn>(found);
    }
    if (syn_resolve("_ZNK10CGPlayer_C6GetBagEv", &found)) {
        api.bag = reinterpret_cast<ItemGuidBagFn>(found);
    }
    if (syn_resolve("_ZNK7CGBag_C14GetItemPointerEi", &found)) {
        api.slot = reinterpret_cast<ItemGuidSlotFn>(found);
    }
    return api;
}

void* item_guid_player() {
    ItemGuidApi& api = item_guid_api();
    if (!api.active || !api.object) {
        return nullptr;
    }
    return api.object(api.active(), 0x10, "test:1", 0xa0);
}

// ʕ •ᴥ•ʔ✿ const GetBag is vtable slot 10 on the container item ✿ ʕ •ᴥ•ʔ
void* item_guid_object_bag(void* item) {
    void* vtable = nullptr;
    memcpy(&vtable, item, sizeof(vtable));
    if (!vtable) {
        return nullptr;
    }
    ItemGuidObjectBagFn get_bag = nullptr;
    memcpy(&get_bag, static_cast<char*>(vtable) + 10 * sizeof(void*), sizeof(get_bag));
    if (!get_bag) {
        return nullptr;
    }
    return get_bag(item);
}

void* item_guid_at(int bag_id, int slot) {
    ItemGuidApi& api = item_guid_api();
    if (!api.bag || !api.slot) {
        return nullptr;
    }
    void* player = item_guid_player();
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
    void* sub = item_guid_object_bag(container);
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

// ʕ •ᴥ•ʔ✿ CGObject::m_obj at 0x10, CGObjectData::m_guid low then high ✿ ʕ •ᴥ•ʔ
int native_get_item_guid(void* state) {
    if (!g_is_num || !g_to_num || !g_push_num || !g_is_num(state, 1) || !g_is_num(state, 2)) {
        return 0;
    }
    int bag_id = item_guid_round(g_to_num(state, 1));
    int slot = item_guid_round(g_to_num(state, 2));
    void* item = item_guid_at(bag_id, slot);
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
    g_push_num(state, static_cast<double>(low));
    g_push_num(state, static_cast<double>(high));
    return 2;
}
