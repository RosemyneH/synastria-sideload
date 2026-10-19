double src_sort_round(double value) {
    if (value >= 0.0) {
        return static_cast<double>(static_cast<int64_t>(value + 0.5));
    }
    return static_cast<double>(static_cast<int64_t>(value - 0.5));
}

bool src_sort_item_id(double value, int* out) {
    constexpr double k_min = -2147483648.0;
    constexpr double k_max = 2147483647.0;
    if (!(value > k_min) || !(value < k_max)) {
        return false;
    }
    double rounded = src_sort_round(value);
    if (!(rounded > k_min) || !(rounded < k_max)) {
        return false;
    }
    int id = static_cast<int>(rounded);
    if (id <= 0 || id >= 0x10000) {
        return false;
    }
    *out = id;
    return true;
}

int src_sort_as_int(double value) {
    constexpr double k_min = -2147483648.0;
    constexpr double k_max = 2147483647.0;
    if (!(value > k_min)) {
        return static_cast<int>(0x80000000u);
    }
    if (!(value < k_max)) {
        return 0x7fffffff;
    }
    double rounded = src_sort_round(value);
    if (!(rounded > k_min)) {
        return static_cast<int>(0x80000000u);
    }
    if (!(rounded < k_max)) {
        return 0x7fffffff;
    }
    return static_cast<int>(rounded);
}

const SrcAtObj& src_sort_obj(const SrcAtSrc& src) {
    if (src.slot < src_at_objs.size()) {
        return src_at_objs[src.slot];
    }
    static const SrcAtObj empty;
    return empty;
}

// ʕ •ᴥ•ʔ✿ 1 type, 2 name, 3 chance, 4 perthousand, 5 zone. Equal keys use the object name ✿ ʕ •ᴥ•ʔ
bool src_sort_before(const SrcAtSrc& a, const SrcAtSrc& b, int mode) {
    if (mode == 1 && a.type != b.type) {
        return a.type < b.type;
    }
    if (mode == 3) {
        float left = static_cast<float>(src_at_chance(a.chance));
        float right = static_cast<float>(src_at_chance(b.chance));
        if (left != right) {
            return left > right;
        }
    } else if (mode == 4) {
        float left = static_cast<float>(src_at_perthousand(a.perthousand));
        float right = static_cast<float>(src_at_perthousand(b.perthousand));
        if (left != right) {
            return left > right;
        }
    } else if (mode == 5) {
        uint16_t left = src_sort_obj(a).zone;
        uint16_t right = src_sort_obj(b).zone;
        if (left != right) {
            return left < right;
        }
    }
    return src_sort_obj(a).name.compare(src_sort_obj(b).name) < 0;
}

int native_itemloc_set_source_sort(void* state) {
    if (!src_at_ready || !g_is_num || !g_to_num || !g_push_num) {
        return 0;
    }
    if (!g_is_num(state, 1) || !g_is_num(state, 2)) {
        return 0;
    }
    int item = 0;
    if (!src_sort_item_id(g_to_num(state, 1), &item)) {
        return 0;
    }
    if (static_cast<size_t>(item) >= src_at_items.size()) {
        return 0;
    }
    std::vector<SrcAtSrc>& sources = src_at_items[static_cast<size_t>(item)];
    if (sources.empty()) {
        return 0;
    }
    int mode = src_sort_as_int(g_to_num(state, 2));
    if (mode == static_cast<int>(src_sort_get_mode[item])) {
        return 0;
    }
    if (mode <= 0 || mode > 5) {
        return 0;
    }
    src_sort_get_mode[item] = static_cast<uint8_t>(mode);
    std::sort(sources.begin(), sources.end(), [mode](const SrcAtSrc& a, const SrcAtSrc& b) {
        return src_sort_before(a, b, mode);
    });
    g_push_num(state, 1.0);
    return 1;
}
