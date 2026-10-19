using HaveTableFn = void (*)(void*, int, int);
using HaveRawFn = void (*)(void*, int);
using HaveCheckFn = int (*)(void*, int);

HaveTableFn have_items_table = nullptr;
HaveRawFn have_items_rawset = nullptr;
HaveCheckFn have_items_check = nullptr;
bool have_items_bound = false;

void have_items_bind() {
    if (have_items_bound) {
        return;
    }
    have_items_bound = true;
    void* found = nullptr;
    if (syn_resolve("lua_createtable", &found)) {
        have_items_table = reinterpret_cast<HaveTableFn>(found);
    }
    found = nullptr;
    if (syn_resolve("lua_rawset", &found)) {
        have_items_rawset = reinterpret_cast<HaveRawFn>(found);
    }
    found = nullptr;
    if (syn_resolve("lua_checkstack", &found)) {
        have_items_check = reinterpret_cast<HaveCheckFn>(found);
    }
}

// ʕ •ᴥ•ʔ✿ positive arg maps id to count; otherwise a 1-based id list from the have-item cache ✿ ʕ •ᴥ•ʔ
int native_get_have_items(void* state) {
    have_items_bind();
    if (!have_items_table || !have_items_rawset || !g_push_num || !g_is_num || !g_to_num) {
        return 0;
    }
    have_items_table(state, 0, 0);
    int id = 0;
    bool dict = g_is_num(state, 1) && have_item_id(g_to_num(state, 1), &id);
    int index = 1;
    for (const auto& entry : g_have_items) {
        if (have_items_check) {
            have_items_check(state, 2);
        }
        uint32_t key = static_cast<uint32_t>(entry.first);
        if (!dict) {
            g_push_num(state, static_cast<double>(index));
            g_push_num(state, static_cast<double>(key));
            ++index;
        } else {
            g_push_num(state, static_cast<double>(key));
            g_push_num(state, static_cast<double>(entry.second));
        }
        have_items_rawset(state, -3);
    }
    return 1;
}
