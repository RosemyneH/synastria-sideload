bool item_vars_id(double n, int* id) {
    constexpr double k_min = -2147483648.0;
    constexpr double k_max = 2147483647.0;
    if (!(n > k_min) || !(n < k_max)) {
        return false;
    }
    double rounded = n + (n >= 0.0 ? 0.5 : -0.5);
    if (!(rounded > k_min) || !(rounded < k_max)) {
        return false;
    }
    int value = static_cast<int>(rounded);
    if (value < 0 || value >= 0x10000) {
        return false;
    }
    *id = value;
    return true;
}

// ʕ •ᴥ•ʔ✿ record bytes +2/+3/+6 are class, subclass, inventory type ✿ ʕ •ᴥ•ʔ
int native_get_item_vars(void* state) {
    if (!g_is_num || !g_to_num || !g_push_num || !g_is_num(state, 1)) {
        return 0;
    }
    int id = 0;
    if (!item_vars_id(g_to_num(state, 1), &id)) {
        return 0;
    }
    const ItemRow* row = item_row(id);
    if (!row) {
        return 0;
    }
    g_push_num(state, static_cast<int8_t>(row->klass));
    g_push_num(state, static_cast<int8_t>(row->sub));
    g_push_num(state, static_cast<int8_t>(row->inv));
    return 3;
}
