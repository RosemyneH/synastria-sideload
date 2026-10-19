int item_name_top() {
    int id = 0xffff;
    while (id > 0) {
        if (item_row(id)) {
            return id;
        }
        --id;
    }
    return 0;
}

// ʕ •ᴥ•ʔ✿ one lua_isstring value. Exact bytes against ItemRow::name, lowest id from 1 through the highest live row. Empty string returns nothing. ✿ ʕ •ᴥ•ʔ
int native_item_id_from_name(void* state) {
    if (!g_is_str || !g_to_str || !g_push_num || !g_is_str(state, 1)) {
        return 0;
    }
    const char* text = g_to_str(state, 1, nullptr);
    if (!text || text[0] == 0) {
        return 0;
    }
    int top = item_name_top();
    for (int id = 1; id <= top; ++id) {
        const ItemRow* row = item_row(id);
        if (!row) {
            continue;
        }
        if (strcmp(row->name.c_str(), text) == 0) {
            g_push_num(state, static_cast<double>(id));
            return 1;
        }
    }
    return 0;
}
