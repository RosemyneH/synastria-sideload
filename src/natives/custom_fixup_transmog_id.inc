int tmog_fix_to_int(double value) {
    constexpr double k_min = -2147483648.0;
    constexpr double k_max = 2147483647.0;
    if (!(value > k_min)) {
        return static_cast<int>(0x80000000u);
    }
    if (!(value < k_max)) {
        return 0x7fffffff;
    }
    double rounded = __builtin_round(value);
    if (!(rounded > k_min)) {
        return static_cast<int>(0x80000000u);
    }
    if (!(rounded < k_max)) {
        return 0x7fffffff;
    }
    return static_cast<int>(rounded);
}

bool tmog_fix_tick(double value) {
    double shifted = value + 0.01;
    if (!(shifted > -2147483648.0) || !(shifted < 2147483647.0)) {
        return true;
    }
    return static_cast<int>(shifted) != 0;
}

// ʕ •ᴥ•ʔ✿ Forge 0..3: category 11 at least 100, or 16/17/20 after adding 0.01 and truncating. Otherwise category 0x1c above 0.5. ✿ ʕ •ᴥ•ʔ
bool tmog_fix_owned(int id) {
    for (int forge = 0; forge <= 3; ++forge) {
        int key = (forge << 24) | id;
        if (game_num(11, key) >= 100.0) {
            return true;
        }
        if (tmog_fix_tick(game_num(16, key)) || tmog_fix_tick(game_num(17, key)) || tmog_fix_tick(game_num(20, key))) {
            return true;
        }
    }
    return game_num(0x1c, id) > 0.5;
}

int tmog_fix_top() {
    int id = 0xffff;
    while (id > 0) {
        if (item_row(id)) {
            return id;
        }
        --id;
    }
    return 0;
}

// ʕ •ᴥ•ʔ✿ ItemRow::display is record +8. An owned id stays. Otherwise the lowest owned id with that display, else the caller id. ✿ ʕ •ᴥ•ʔ
int tmog_fix_id(int id) {
    if (id <= 0 || id > 0xffff) {
        return id;
    }
    const ItemRow* row = item_row(id);
    if (!row || row->display == 0) {
        return id;
    }
    if (tmog_fix_owned(id)) {
        return id;
    }
    uint32_t display = row->display;
    int top = tmog_fix_top();
    for (int cand = 1; cand <= top; ++cand) {
        const ItemRow* other = item_row(cand);
        if (!other || other->display != display) {
            continue;
        }
        if (tmog_fix_owned(cand)) {
            return cand;
        }
    }
    return id;
}

int native_fixup_transmog_id(void* state) {
    if (!g_is_num || !g_to_num || !g_push_num) {
        return 0;
    }
    int id = 0;
    if (g_is_num(state, 1)) {
        id = tmog_fix_to_int(g_to_num(state, 1));
    }
    g_push_num(state, static_cast<double>(tmog_fix_id(id)));
    return 1;
}
