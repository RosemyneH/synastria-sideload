using dress_set_type_fn = int (*)();
using dress_set_this_fn = void* (*)(void* state, int type);

struct dress_set_api {
    dress_set_type_fn type = nullptr;
    dress_set_this_fn object = nullptr;
    bool bound = false;
};

dress_set_api& dress_set_fns() {
    static dress_set_api api;
    if (api.bound) {
        return api;
    }
    api.bound = true;
    void* found = nullptr;
    if (syn_resolve("_ZN12CSimpleModel13GetObjectTypeEv", &found)) {
        api.type = reinterpret_cast<dress_set_type_fn>(found);
    }
    if (syn_resolve("_Z25FrameScript_GetObjectThisP9lua_Statei", &found)) {
        api.object = reinterpret_cast<dress_set_this_fn>(found);
    }
    return api;
}

// ʕ •ᴥ•ʔ✿ dressup_facing maps the CM2Model world matrix (frame+0x460, then +0x140). Zero erases the entry; clear drops the map. ✿ ʕ •ᴥ•ʔ
void* dress_set_key(void* frame) {
    if (!frame) {
        return nullptr;
    }
    void* model = *reinterpret_cast<void**>(static_cast<char*>(frame) + 0x460);
    if (!model) {
        return nullptr;
    }
    return static_cast<char*>(model) + 0x140;
}

int native_set_dressup_facing(void* state) {
    dress_set_api& api = dress_set_fns();
    if (!api.type || !api.object) {
        return 0;
    }
    void* frame = api.object(state, api.type());
    if (!frame) {
        return 0;
    }
    if (!g_is_num || !g_to_num || !g_is_num(state, 2)) {
        return 0;
    }
    float facing = static_cast<float>(g_to_num(state, 2));
    void* key = dress_set_key(frame);
    if (!key) {
        return 0;
    }
    if (facing == 0.0f) {
        dressup_facing.erase(key);
    } else {
        dressup_facing[key] = facing;
    }
    return 0;
}
