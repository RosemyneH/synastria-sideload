void* equip_mgr_sets() {
    static void* list = nullptr;
    static bool ready = false;
    if (!ready) {
        ready = true;
        syn_resolve("_ZN18CGEquipmentManager15s_equipmentSetsE", &list);
    }
    return list;
}

// ʕ •ᴥ•ʔ✿ TSList head is terminator.m_next; a node is prev, next, EQUIPMENT_SET* ✿ ʕ •ᴥ•ʔ
bool equip_mgr_has_guid(uint32_t low, uint32_t high) {
    void* list = equip_mgr_sets();
    if (!list) {
        return false;
    }
    void* node = nullptr;
    memcpy(&node, static_cast<char*>(list) + 2 * sizeof(void*), sizeof(node));
    while (node && (reinterpret_cast<uintptr_t>(node) & 1u) == 0) {
        void* set = nullptr;
        memcpy(&set, static_cast<char*>(node) + 2 * sizeof(void*), sizeof(set));
        if (set) {
            for (int slot = 0; slot < 19; ++slot) {
                uint32_t item_low = 0;
                uint32_t item_high = 0;
                char* guid = static_cast<char*>(set) + 0x198 + slot * 8;
                memcpy(&item_low, guid, sizeof(item_low));
                memcpy(&item_high, guid + 4, sizeof(item_high));
                if (item_low == low && item_high == high) {
                    return true;
                }
            }
        }
        memcpy(&node, static_cast<char*>(node) + sizeof(void*), sizeof(node));
    }
    return false;
}

// ʕ •ᴥ•ʔ✿ bag, slot. Item guid against every EQUIPMENT_SET::m_items. Pushes 1 or nothing. ✿ ʕ •ᴥ•ʔ
int native_is_item_equip_mgr(void* state) {
    if (!g_is_num || !g_to_num || !g_push_num || !g_is_num(state, 1) || !g_is_num(state, 2)) {
        return 0;
    }
    int bag = item_count_round(g_to_num(state, 1));
    int slot = item_count_round(g_to_num(state, 2));
    void* item = item_count_at(bag, slot);
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
    if (low == 0 && high == 0) {
        return 0;
    }
    if (!equip_mgr_has_guid(low, high)) {
        return 0;
    }
    g_push_num(state, 1.0);
    return 1;
}
