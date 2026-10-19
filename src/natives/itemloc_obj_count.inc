// ʕ •ᴥ•ʔ✿ Same rows as ItemLocGetObjAt. Zone-name keys (high byte 5) are not a count. ✿ ʕ •ᴥ•ʔ
int native_itemloc_obj_count(void* state) {
    if (!g_is_num || !g_to_num || !g_is_num(state, 1) || !g_is_num(state, 2)) {
        return 0;
    }
    int cat = obj_at_to_int(g_to_num(state, 1));
    int id = obj_at_to_int(g_to_num(state, 2));
    uint32_t key = (static_cast<uint32_t>(cat) << 24) | static_cast<uint32_t>(id);
    if ((key >> 24) == 5) {
        return 0;
    }
    auto found = obj_at_rows.find(key);
    if (found == obj_at_rows.end() || found->second.empty()) {
        return 0;
    }
    g_push_num(state, static_cast<double>(found->second.size()));
    return 1;
}
