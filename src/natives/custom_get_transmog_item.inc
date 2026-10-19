struct transmog_rec {
    int id;
    const char* name;
};

// ʕ •ᴥ•ʔ✿ transmog is the shared 8-byte list: int32 id, then const char* name. Ids 0 and 1 lead as equipped and hidden. A negative id is the selected item the account has not acquired. Names are ItemRow::name and order only the ids above 1. ✿ ʕ •ᴥ•ʔ
std::vector<transmog_rec> transmog;

int tmog_item_int(double n) {
    constexpr double k_min = -2147483648.0;
    constexpr double k_max = 2147483647.0;
    if (!(n > k_min)) {
        return static_cast<int>(0x80000000u);
    }
    if (!(n < k_max)) {
        return 0x7fffffff;
    }
    double rounded = __builtin_round(n);
    if (!(rounded > k_min)) {
        return static_cast<int>(0x80000000u);
    }
    if (!(rounded < k_max)) {
        return 0x7fffffff;
    }
    return static_cast<int>(rounded);
}

int tmog_item_upper(unsigned char c) {
    if (c >= 'a' && c <= 'z') {
        return c - 32;
    }
    return c;
}

int tmog_item_icmp(const char* left, const char* right) {
    const unsigned char* a = reinterpret_cast<const unsigned char*>(left ? left : "");
    const unsigned char* b = reinterpret_cast<const unsigned char*>(right ? right : "");
    while (*a && tmog_item_upper(*a) == tmog_item_upper(*b)) {
        ++a;
        ++b;
    }
    return tmog_item_upper(*a) - tmog_item_upper(*b);
}

bool tmog_item_before(const transmog_rec& left, const transmog_rec& right) {
    if (left.id > 1 && right.id > 1) {
        return tmog_item_icmp(left.name, right.name) < 0;
    }
    return left.id < right.id;
}

bool tmog_item_has(const char* hay, const char* needle) {
    const unsigned char* h = reinterpret_cast<const unsigned char*>(hay ? hay : "");
    const unsigned char* n = reinterpret_cast<const unsigned char*>(needle);
    size_t nlen = strlen(reinterpret_cast<const char*>(n));
    size_t hlen = strlen(reinterpret_cast<const char*>(h));
    if (hlen < nlen) {
        return false;
    }
    size_t last = hlen - nlen;
    for (size_t i = 0; i <= last; ++i) {
        size_t j = 0;
        while (j < nlen && tmog_item_upper(h[i + j]) == tmog_item_upper(n[j])) {
            ++j;
        }
        if (j == nlen) {
            return true;
        }
    }
    return false;
}

bool tmog_item_owned(int id) {
    for (int forge = 0; forge <= 3; ++forge) {
        int key = (forge << 24) | id;
        if (game_num(11, key) >= 100.0) {
            return true;
        }
        if (static_cast<int>(game_num(16, key) + 0.01) != 0) {
            return true;
        }
        if (static_cast<int>(game_num(17, key) + 0.01) != 0) {
            return true;
        }
        if (static_cast<int>(game_num(20, key) + 0.01) != 0) {
            return true;
        }
    }
    return game_num(0x1c, id) > 0.5;
}

bool tmog_item_class_ok(const ItemRow* row, int filter, int id, int selected) {
    if (filter == 0 || id == selected) {
        return true;
    }
    int sub_f = filter & 0xff;
    int cls_f = (static_cast<unsigned>(filter) >> 8) & 0xff;
    uint8_t klass = row->klass;
    uint8_t sub = row->sub;
    if (klass == 4 && sub == 5) {
        sub = 6;
    }
    int cls = static_cast<int8_t>(static_cast<uint8_t>(klass + 1));
    int sb = static_cast<int8_t>(static_cast<uint8_t>(sub + 1));
    if (cls_f != 0 && cls != cls_f) {
        return false;
    }
    if (sub_f != 0 && sb != sub_f) {
        return false;
    }
    return true;
}

bool tmog_item_inv(int slot, uint8_t inv) {
    switch (slot) {
    case 0: return inv == 1;
    case 1: return inv == 3;
    case 2: return inv == 16;
    case 3: return inv == 5 || inv == 20;
    case 4: return inv == 4;
    case 5: return inv == 19;
    case 6: return inv == 9;
    case 7: return inv == 10;
    case 8: return inv == 6;
    case 9: return inv == 7;
    case 10: return inv == 8;
    case 11: return inv == 13 || inv == 17 || inv == 21 || inv == 22;
    case 12: return inv == 13 || inv == 17 || inv == 21 || inv == 22 || inv == 23 || inv == 14;
    case 13: return inv == 15 || inv == 25 || inv == 26;
    default: return false;
    }
}

int tmog_item_top() {
    int id = 0xffff;
    while (id > 0) {
        if (item_row(id)) {
            return id;
        }
        --id;
    }
    return 0;
}

int tmog_item_build(void* state) {
    transmog.clear();
    int selected = 0;
    if (g_is_num(state, 3)) {
        selected = tmog_item_int(g_to_num(state, 3));
    }
    if (!g_is_num(state, 2)) {
        g_push_num(state, 0);
        g_push_num(state, 0);
        return 2;
    }
    int slot = tmog_item_int(g_to_num(state, 2)) - 1;
    if (static_cast<unsigned>(slot) > 13u) {
        g_push_num(state, 0);
        g_push_num(state, 0);
        return 2;
    }
    int filter = 0;
    if (g_is_num(state, 4)) {
        filter = tmog_item_int(g_to_num(state, 4));
    }
    const char* text = nullptr;
    if (g_is_str && g_to_str && g_is_str(state, 5)) {
        text = g_to_str(state, 5, nullptr);
        if (text && text[0] == 0) {
            text = nullptr;
        }
    }
    int flags = 0;
    if (g_is_num(state, 6)) {
        flags = tmog_item_int(g_to_num(state, 6));
    }

    transmog.push_back({0, ""});
    transmog.push_back({1, ""});
    bool dedup = (flags & 4) == 0 && text == nullptr;
    std::set<uint32_t> seen;
    int top = tmog_item_top();
    for (int id = 1; id <= top; ++id) {
        const ItemRow* row = item_row(id);
        if (!row || row->inv == 0) {
            continue;
        }
        if (!tmog_item_class_ok(row, filter, id, selected) || !tmog_item_inv(slot, row->inv)) {
            continue;
        }
        bool owned = tmog_item_owned(id);
        if (id != selected && !owned) {
            continue;
        }
        if (id != selected && text && !tmog_item_has(row->name.c_str(), text)) {
            continue;
        }
        if (dedup) {
            bool fresh = seen.insert(row->display).second;
            if (id != selected && !fresh) {
                continue;
            }
        }
        transmog.push_back({owned ? id : -id, row->name.c_str()});
    }
    if (transmog.size() > 1) {
        std::sort(transmog.begin(), transmog.end(), tmog_item_before);
    }

    int pick = -1;
    for (size_t i = 0; i < transmog.size(); ++i) {
        int id = transmog[i].id;
        if (id < 0) {
            id = -id;
        }
        if (id == selected) {
            pick = static_cast<int>(i);
            break;
        }
    }
    g_push_num(state, static_cast<double>(static_cast<int>(transmog.size())));
    g_push_num(state, static_cast<double>(pick + 1));
    return 2;
}

int native_get_transmog_item(void* state) {
    if (!g_is_num || !g_to_num || !g_push_num || !g_is_num(state, 1)) {
        return 0;
    }
    int mode = tmog_item_int(g_to_num(state, 1));
    if (mode == 0) {
        return tmog_item_build(state);
    }
    if (mode != 1 || !g_is_num(state, 2)) {
        return 0;
    }
    int index = tmog_item_int(g_to_num(state, 2)) - 1;
    if (index < 0 || index >= static_cast<int>(transmog.size())) {
        return 0;
    }
    g_push_num(state, static_cast<double>(transmog[static_cast<size_t>(index)].id));
    return 1;
}
