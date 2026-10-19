using CacheActiveFn = uint64_t (*)();
using CacheObjectFn = void* (*)(uint64_t guid, uint32_t mask, const char* file, int line);
using CacheBagFn = void* (*)(void* player);
using CacheSlotFn = void* (*)(void* bag, int slot);
using CacheFindFn = void* (*)(void* bag, int (*visit)(void* item, void* param), void* param, uint64_t* container, uint32_t* slot, uint32_t flags);

struct CacheHaveApi {
    CacheActiveFn active = nullptr;
    CacheObjectFn object = nullptr;
    CacheBagFn bag = nullptr;
    CacheSlotFn slot = nullptr;
    CacheFindFn find = nullptr;
    bool bound = false;
};

CacheHaveApi& cache_have_api() {
    static CacheHaveApi api;
    if (api.bound) {
        return api;
    }
    api.bound = true;
    void* found = nullptr;
    if (syn_resolve("_Z25ClntObjMgrGetActivePlayerv", &found)) {
        api.active = reinterpret_cast<CacheActiveFn>(found);
    }
    if (syn_resolve("_Z19ClntObjMgrObjectPtr7WOWGUID8TypeMaskPKci", &found)) {
        api.object = reinterpret_cast<CacheObjectFn>(found);
    }
    if (syn_resolve("_ZNK10CGPlayer_C6GetBagEv", &found)) {
        api.bag = reinterpret_cast<CacheBagFn>(found);
    }
    if (syn_resolve("_ZNK7CGBag_C14GetItemPointerEi", &found)) {
        api.slot = reinterpret_cast<CacheSlotFn>(found);
    }
    if (syn_resolve("_ZN7CGBag_C8FindItemEPFiPK8CGItem_CPvES3_R7WOWGUIDRjj", &found)) {
        api.find = reinterpret_cast<CacheFindFn>(found);
    }
    return api;
}

void* cache_have_bag() {
    CacheHaveApi& api = cache_have_api();
    if (!api.active || !api.object || !api.bag) {
        return nullptr;
    }
    void* player = api.object(api.active(), 0x10, "test:1", 0xa0);
    if (!player) {
        return nullptr;
    }
    return api.bag(player);
}

// ʕ •ᴥ•ʔ✿ DLL m_obj +0x8 and m_item +0xd4. Linux CGItem_C is +0x10 and +0x168. Entry +0xc, stack +0x20, else 1. ✿ ʕ •ᴥ•ʔ
void cache_have_add(void* item) {
    void* fields = nullptr;
    memcpy(&fields, static_cast<char*>(item) + 0x10, sizeof(fields));
    if (!fields) {
        return;
    }
    int32_t entry = 0;
    memcpy(&entry, static_cast<char*>(fields) + 0xc, sizeof(entry));
    if (entry <= 0) {
        return;
    }
    int32_t stack = 1;
    void* data = nullptr;
    memcpy(&data, static_cast<char*>(item) + 0x168, sizeof(data));
    if (data) {
        int32_t count = 0;
        memcpy(&count, static_cast<char*>(data) + 0x20, sizeof(count));
        if (count > 0) {
            stack = count;
        }
    }
    g_have_items[entry] += stack;
}

int cache_have_visit(void* item, void* param) {
    (void)param;
    if (item) {
        cache_have_add(item);
    }
    return 0;
}

// ʕ •ᴥ•ʔ✿ Slots 0-18, then FindItem flags 8. That visit returns 0 so containers are walked. Equipped slots are summed twice. ✿ ʕ •ᴥ•ʔ
int native_cache_have_items(void* state) {
    (void)state;
    g_have_items.clear();
    CacheHaveApi& api = cache_have_api();
    void* bag = cache_have_bag();
    if (bag && api.slot) {
        for (int slot = 0; slot < 0x13; ++slot) {
            void* item = api.slot(bag, slot);
            if (item) {
                cache_have_add(item);
            }
        }
    }
    if (bag && api.find) {
        uint64_t container = 0;
        uint32_t slot = 0;
        api.find(bag, cache_have_visit, nullptr, &container, &slot, 8u);
    }
    log_line("cache have items %zu", g_have_items.size());
    return 0;
}
