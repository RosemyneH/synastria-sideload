using ContainerSoulActiveFn = uint64_t (*)();
using ContainerSoulObjectFn = void* (*)(uint64_t guid, uint32_t mask, const char* file, int line);
using ContainerSoulBagFn = void* (*)(void* player);
using ContainerSoulAtFn = uint64_t (*)(uint32_t index);
using ContainerSoulSlotFn = void* (*)(void* bag, int slot);
using ContainerSoulBoundFn = bool (*)(void* item);
using ContainerSoulVBagFn = void* (*)(void* item);

struct ContainerSoulApi {
    ContainerSoulActiveFn active = nullptr;
    ContainerSoulObjectFn object = nullptr;
    ContainerSoulBagFn bag = nullptr;
    ContainerSoulAtFn at = nullptr;
    ContainerSoulSlotFn slot = nullptr;
    ContainerSoulBoundFn bound = nullptr;
    bool ready = false;
};

ContainerSoulApi& container_soul_api() {
    static ContainerSoulApi api;
    if (api.ready) {
        return api;
    }
    api.ready = true;
    void* found = nullptr;
    if (syn_resolve("_Z25ClntObjMgrGetActivePlayerv", &found)) {
        api.active = reinterpret_cast<ContainerSoulActiveFn>(found);
    }
    if (syn_resolve("_Z19ClntObjMgrObjectPtr7WOWGUID8TypeMaskPKci", &found)) {
        api.object = reinterpret_cast<ContainerSoulObjectFn>(found);
    }
    if (syn_resolve("_ZNK10CGPlayer_C6GetBagEv", &found)) {
        api.bag = reinterpret_cast<ContainerSoulBagFn>(found);
    }
    if (syn_resolve("_Z13GetBagAtIndexj", &found)) {
        api.at = reinterpret_cast<ContainerSoulAtFn>(found);
    }
    if (syn_resolve("_ZNK7CGBag_C14GetItemPointerEi", &found)) {
        api.slot = reinterpret_cast<ContainerSoulSlotFn>(found);
    }
    if (syn_resolve("_ZN8CGItem_C7IsBoundEv", &found)) {
        api.bound = reinterpret_cast<ContainerSoulBoundFn>(found);
    }
    return api;
}

int32_t container_soul_int(double n) {
    if (n != n || n < -2147483648.0 || n >= 2147483648.0) {
        return static_cast<int32_t>(0x80000000u);
    }
    return static_cast<int32_t>(n);
}

// ʕ •ᴥ•ʔ✿ const GetBag is vtable slot 10 on the container item ✿ ʕ •ᴥ•ʔ
void* container_soul_object_bag(void* item) {
    void* vtable = nullptr;
    memcpy(&vtable, item, sizeof(vtable));
    if (!vtable) {
        return nullptr;
    }
    ContainerSoulVBagFn get_bag = nullptr;
    memcpy(&get_bag, static_cast<char*>(vtable) + 10 * sizeof(void*), sizeof(get_bag));
    if (!get_bag) {
        return nullptr;
    }
    return get_bag(item);
}

void* container_soul_item(void* state) {
    ContainerSoulApi& api = container_soul_api();
    if (!api.active || !api.object || !api.bag || !api.at || !api.slot) {
        return nullptr;
    }
    void* player = api.object(api.active(), 0x10, "test:1", 0xa0);
    if (!player) {
        return nullptr;
    }
    int32_t index = static_cast<int32_t>(static_cast<uint32_t>(container_soul_int(g_to_num(state, 1))) - 1u);
    int32_t slot = static_cast<int32_t>(static_cast<uint32_t>(container_soul_int(g_to_num(state, 2))) - 1u);
    void* bag = nullptr;
    if (index < 0) {
        bag = api.bag(player);
        uint32_t key = static_cast<uint32_t>(index) + 5u;
        if (key <= 4u) {
            if (key == 0) {
                return nullptr;
            }
            if (key == 2) {
                slot = static_cast<int32_t>(static_cast<uint32_t>(slot) + 0x56u);
                if (slot > 0x75) {
                    return nullptr;
                }
            } else if (key == 3) {
                slot = static_cast<int32_t>(static_cast<uint32_t>(slot) + 0x27u);
                if (slot > 0x42) {
                    return nullptr;
                }
            } else if (key == 4) {
                slot = static_cast<int32_t>(static_cast<uint32_t>(slot) + 0x17u);
                if (slot > 0x26) {
                    return nullptr;
                }
            }
        }
    } else if (index < 11) {
        void* container = api.object(api.at(static_cast<uint32_t>(index)), 4, ".\\ContainerFrame.cpp", 0x1fd);
        if (container) {
            bag = container_soul_object_bag(container);
        }
    }
    if (!bag) {
        return nullptr;
    }
    int32_t num = 0;
    memcpy(&num, bag, sizeof(num));
    if (slot < 0 || slot >= num) {
        return nullptr;
    }
    return api.slot(bag, slot);
}

// ʕ •ᴥ•ʔ✿ lua bag, slot. 0 backpack 23-38, -1 bank 39-66, -2 keyring 86-117, 1-11 containers. IsBound pushes 1 or nothing. ✿ ʕ •ᴥ•ʔ
int native_is_container_soulbound(void* state) {
    if (!g_is_num || !g_to_num || !g_push_num || !g_is_num(state, 1) || !g_is_num(state, 2)) {
        return 0;
    }
    ContainerSoulApi& api = container_soul_api();
    void* item = container_soul_item(state);
    if (!item || !api.bound || !api.bound(item)) {
        return 0;
    }
    g_push_num(state, 1.0);
    return 1;
}
