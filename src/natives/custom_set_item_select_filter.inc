std::vector<item_select_rec> item_select_rows;

// ʕ •ᴥ•ʔ✿ GetItemSelect and GetItemSelectCount read item_select. Each pointer aims at an item_select_rows record: name is ItemRow::name, id is the item. Rows fill once and sort by _stricmp then id. A search keeps at most 101 StrStrIA hits. ✿ ʕ •ᴥ•ʔ

int item_select_icmp(const char* left, const char* right) {
    if (!left) {
        left = "";
    }
    if (!right) {
        right = "";
    }
    while (*left && *right) {
        unsigned char a = static_cast<unsigned char>(*left);
        unsigned char b = static_cast<unsigned char>(*right);
        if (a >= 'A' && a <= 'Z') {
            a = static_cast<unsigned char>(a + 32);
        }
        if (b >= 'A' && b <= 'Z') {
            b = static_cast<unsigned char>(b + 32);
        }
        if (a != b) {
            return a < b ? -1 : 1;
        }
        ++left;
        ++right;
    }
    if (*left == *right) {
        return 0;
    }
    return *left ? 1 : -1;
}

const char* item_select_istr(const char* hay, const char* needle) {
    if (!hay || !needle || !needle[0]) {
        return nullptr;
    }
    for (const char* at = hay; *at; ++at) {
        const char* left = at;
        const char* right = needle;
        while (*left && *right) {
            unsigned char a = static_cast<unsigned char>(*left);
            unsigned char b = static_cast<unsigned char>(*right);
            if (a >= 'A' && a <= 'Z') {
                a = static_cast<unsigned char>(a + 32);
            }
            if (b >= 'A' && b <= 'Z') {
                b = static_cast<unsigned char>(b + 32);
            }
            if (a != b) {
                break;
            }
            ++left;
            ++right;
        }
        if (*right == 0) {
            return at;
        }
    }
    return nullptr;
}

bool item_select_less(const item_select_rec& left, const item_select_rec& right) {
    int order = item_select_icmp(left.name, right.name);
    if (order == 0) {
        return left.id < right.id;
    }
    return order < 0;
}

int item_select_top() {
    int id = 0xffff;
    while (id > 0) {
        if (item_row(id)) {
            return id;
        }
        --id;
    }
    return 0;
}

void item_select_ensure() {
    if (!item_select_rows.empty()) {
        return;
    }
    int top = item_select_top();
    for (int id = 1; id <= top; ++id) {
        const ItemRow* row = item_row(id);
        if (!row) {
            continue;
        }
        item_select_rows.push_back({row->name.c_str(), static_cast<uint32_t>(id)});
    }
    if (item_select_rows.size() > 1) {
        std::sort(item_select_rows.begin(), item_select_rows.end(), item_select_less);
    }
}

int native_set_item_select_filter(void* state) {
    item_select_ensure();
    if (!g_is_str || !g_to_str || !g_is_str(state, 1)) {
        return 0;
    }
    const char* text = g_to_str(state, 1, nullptr);
    if (!text || text[0] == 0) {
        item_select.clear();
        return 0;
    }
    item_select.clear();
    int left = 0x64;
    for (item_select_rec& rec : item_select_rows) {
        if (!item_select_istr(rec.name, text)) {
            continue;
        }
        item_select.push_back(&rec);
        if (--left < 0) {
            break;
        }
    }
    return 0;
}
