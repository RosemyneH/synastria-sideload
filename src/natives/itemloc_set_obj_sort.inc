struct obj_sort_keep {
    const obj_at_row* data = nullptr;
    uint8_t mode = 0;
};

std::map<uint32_t, obj_sort_keep> obj_sort_kept;

// ʕ •ᴥ•ʔ✿ A missing item record makes the pair unordered, same as a failed id lookup ✿ ʕ •ᴥ•ʔ
bool obj_sort_name_less(uint16_t a, uint16_t b) {
    const ItemRow* left = item_row(a);
    const ItemRow* right = item_row(b);
    if (!left || !right) {
        return false;
    }
    return left->name < right->name;
}

// ʕ •ᴥ•ʔ✿ 1 name, 2 chance, 3 perthousand. Numeric keys are higher first, then the item name ✿ ʕ •ᴥ•ʔ
bool obj_sort_before(const obj_at_row& a, const obj_at_row& b, int mode) {
    if (mode == 2) {
        float left = static_cast<float>(obj_at_chance(a.chance));
        float right = static_cast<float>(obj_at_chance(b.chance));
        if (left != right) {
            return left > right;
        }
    } else if (mode == 3) {
        float left = static_cast<float>(obj_at_pert(a.pert));
        float right = static_cast<float>(obj_at_pert(b.pert));
        if (left != right) {
            return left > right;
        }
    }
    return obj_sort_name_less(a.item, b.item);
}

int native_itemloc_set_obj_sort(void* state) {
    if (!g_is_num || !g_to_num || !g_push_num) {
        return 0;
    }
    if (!g_is_num(state, 1) || !g_is_num(state, 2) || !g_is_num(state, 3)) {
        return 0;
    }
    int cat = obj_at_to_int(g_to_num(state, 1));
    int id = obj_at_to_int(g_to_num(state, 2));
    int mode = obj_at_to_int(g_to_num(state, 3));
    uint32_t key = (static_cast<uint32_t>(cat) << 24) | static_cast<uint32_t>(id);
    auto found = obj_at_rows.find(key);
    if (found == obj_at_rows.end() || found->second.empty()) {
        return 0;
    }
    if (mode <= 0 || mode > 3) {
        return 0;
    }
    std::vector<obj_at_row>& rows = found->second;
    obj_sort_keep& keep = obj_sort_kept[key];
    if (keep.data != rows.data()) {
        keep.data = rows.data();
        keep.mode = 0;
    }
    if (mode == static_cast<int>(keep.mode)) {
        return 0;
    }
    keep.mode = static_cast<uint8_t>(mode);
    std::sort(rows.begin(), rows.end(), [mode](const obj_at_row& a, const obj_at_row& b) {
        return obj_sort_before(a, b, mode);
    });
    g_push_num(state, 1.0);
    return 1;
}
