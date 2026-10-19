using CursorItemFn = uint64_t (*)();
using CursorObjectFn = void* (*)(uint64_t guid, uint32_t mask, const char* file, int line);

struct CursorItemApi {
    int32_t* type = nullptr;
    uint64_t* item = nullptr;
    CursorItemFn get = nullptr;
    CursorObjectFn object = nullptr;
    bool bound = false;
};

CursorItemApi& cursor_item_api() {
    static CursorItemApi api;
    if (api.bound) {
        return api;
    }
    api.bound = true;
    void* found = nullptr;
    if (syn_resolve("_ZN8CGGameUI16m_cursorItemTypeE", &found)) {
        api.type = static_cast<int32_t*>(found);
    }
    if (syn_resolve("_ZN8CGGameUI12m_cursorItemE", &found)) {
        api.item = static_cast<uint64_t*>(found);
    }
    if (syn_resolve("_ZN8CGGameUI13GetCursorItemEv", &found)) {
        api.get = reinterpret_cast<CursorItemFn>(found);
    }
    if (syn_resolve("_Z19ClntObjMgrObjectPtr7WOWGUID8TypeMaskPKci", &found)) {
        api.object = reinterpret_cast<CursorObjectFn>(found);
    }
    return api;
}

// ʕ •ᴥ•ʔ✿ CGItem_C::m_item at 0x168, CGItemData::m_itemFieldStackCount at 0x20 ✿ ʕ •ᴥ•ʔ
int32_t cursor_place_count(void* item) {
    void* data = nullptr;
    memcpy(&data, static_cast<char*>(item) + 0x168, sizeof(data));
    if (!data) {
        return 1;
    }
    int32_t count = 0;
    memcpy(&count, static_cast<char*>(data) + 0x20, sizeof(count));
    return count;
}

int native_get_cursor_item(void* state) {
    CursorItemApi& api = cursor_item_api();
    if (!api.type || !api.item || !api.get || !api.object || !g_push_num) {
        return 0;
    }
    // ʕ •ᴥ•ʔ✿ UICURSOR_ITEM; otherwise the native pushes nothing ✿ ʕ •ᴥ•ʔ
    if (*api.type != 1) {
        return 0;
    }
    void* item = api.object(api.get(), 2, ".\\GameUI.cpp", 0x5dd);
    if (!item) {
        return 0;
    }
    uint64_t guid = *api.item;
    g_push_num(state, static_cast<double>(static_cast<uint32_t>(guid)));
    g_push_num(state, static_cast<double>(static_cast<uint32_t>(guid >> 32)));
    g_push_num(state, static_cast<double>(cursor_place_count(item)));
    return 3;
}
