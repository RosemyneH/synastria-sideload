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

// ʕ •ᴥ•ʔ✿ Linux CSimpleModel: m_model at 0x2A0, m_facing at 0x328. The 32-bit slot 0x460 is past the object. ✿ ʕ •ᴥ•ʔ
void dress_set_apply(void* frame, float facing) {
    auto* bytes = static_cast<char*>(frame);
    memcpy(bytes + 0x328, &facing, sizeof(facing));
    if (facing == 0.0f) {
        dressup_facing.erase(frame);
        return;
    }
    dressup_facing[frame] = facing;
}

int native_set_dressup_facing(void* state) {
    dress_set_api& api = dress_set_fns();
    if (!api.type || !api.object || !g_type || g_type(state, 1) != 5) {
        return 0;
    }
    void* frame = api.object(state, api.type());
    if (!frame) {
        return 0;
    }
    if (!g_is_num || !g_to_num || !g_is_num(state, 2)) {
        return 0;
    }
    dress_set_apply(frame, static_cast<float>(g_to_num(state, 2)));
    return 0;
}
