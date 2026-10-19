int near_map_id(double n) {
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

struct NearPos {
    float x;
    float y;
    float z;
};

using NearMapFn = uint32_t (*)();
using NearActiveFn = uint64_t (*)();
using NearObjectFn = void* (*)(uint64_t guid, uint32_t mask, const char* file, int line);
using NearPosFn = NearPos (*)(const void* unit);

struct NearApi {
    NearMapFn map = nullptr;
    NearActiveFn active = nullptr;
    NearObjectFn object = nullptr;
    NearPosFn position = nullptr;
    bool bound = false;
};

NearApi& near_api() {
    static NearApi api;
    if (api.bound) {
        return api;
    }
    api.bound = true;
    void* found = nullptr;
    if (syn_resolve("_Z18ClntObjMgrGetMapIDv", &found)) {
        api.map = reinterpret_cast<NearMapFn>(found);
    }
    if (syn_resolve("_Z25ClntObjMgrGetActivePlayerv", &found)) {
        api.active = reinterpret_cast<NearActiveFn>(found);
    }
    if (syn_resolve("_Z19ClntObjMgrObjectPtr7WOWGUID8TypeMaskPKci", &found)) {
        api.object = reinterpret_cast<NearObjectFn>(found);
    }
    if (syn_resolve("_ZNK8CGUnit_C11GetPositionEv", &found)) {
        api.position = reinterpret_cast<NearPosFn>(found);
    }
    return api;
}

// ʕ •ᴥ•ʔ✿ map, x, y, optional z, optional radius (30). Map must match ClntObjMgrGetMapID. Distance is against CGUnit_C::GetPosition on the active player. Pushes 1 when radius^2 is strictly greater. ✿ ʕ •ᴥ•ʔ
int native_is_player_near(void* state) {
    if (!g_is_num || !g_to_num || !g_push_num) {
        return 0;
    }
    if (!g_is_num(state, 1) || !g_is_num(state, 2) || !g_is_num(state, 3)) {
        return 0;
    }
    NearApi& api = near_api();
    if (!api.map || near_map_id(g_to_num(state, 1)) != static_cast<int>(api.map())) {
        return 0;
    }
    double x = g_to_num(state, 2);
    double y = g_to_num(state, 3);
    bool have_z = g_is_num(state, 4);
    double z = have_z ? g_to_num(state, 4) : 0.0;
    double radius = 30.0;
    if (g_is_num(state, 5)) {
        radius = g_to_num(state, 5);
        if (!(radius > 0.0)) {
            return 0;
        }
    }
    NearPos pos{};
    if (api.active && api.object && api.position) {
        void* player = api.object(api.active(), 0x10, "test:1", 0xa0);
        if (player) {
            pos = api.position(player);
        }
    }
    double dx = x - static_cast<double>(pos.x);
    double dy = y - static_cast<double>(pos.y);
    double dist2 = dx * dx + dy * dy;
    if (have_z) {
        double dz = z - static_cast<double>(pos.z);
        dist2 += dz * dz;
    }
    if (!(radius * radius > dist2)) {
        return 0;
    }
    g_push_num(state, 1.0);
    return 1;
}
