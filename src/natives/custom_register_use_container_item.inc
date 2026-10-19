// ʕ •ᴥ•ʔ✿ use_container_hooks records: name, handler (" becomes \"), value (higher sorts first, then name), mode 0 or 1. use_container_dispatching is the walk flag. use_container_pending queues new hooks, and empty-handler records mean unregister, until the walk ends ✿ ʕ •ᴥ•ʔ

int use_reg_debug = 0;

bool use_reg_before(const UseContainerHook* a, const UseContainerHook* b) {
    if (a->value > b->value) {
        return true;
    }
    if (b->value > a->value) {
        return false;
    }
    return a->name < b->name;
}

void use_reg_sort() {
    std::sort(use_container_hooks.begin(), use_container_hooks.end(), use_reg_before);
}

std::string use_reg_escape(const char* text) {
    std::string out;
    for (const char* p = text; *p; ++p) {
        if (*p == '"') {
            out.push_back('\\');
        }
        out.push_back(*p);
    }
    return out;
}

int use_reg_mode(double value) {
    constexpr double k_min = -2147483648.0;
    constexpr double k_max = 2147483647.0;
    if (!(value > k_min)) {
        return 0;
    }
    if (!(value < k_max)) {
        return 1;
    }
    double rounded = __builtin_round(value);
    if (!(rounded > k_min)) {
        return 0;
    }
    if (!(rounded < k_max)) {
        return 1;
    }
    int n = static_cast<int>(rounded);
    if (n < 0) {
        return 0;
    }
    if (n <= 1) {
        return n;
    }
    return 1;
}

UseContainerHook* use_reg_find(const char* name) {
    for (UseContainerHook* hook : use_container_hooks) {
        if (hook->name == name) {
            return hook;
        }
    }
    return nullptr;
}

bool use_reg_debug_on() {
    if (use_reg_debug == 0) {
        FILE* file = fopen("WoWExt.debug.txt", "r");
        use_reg_debug = file ? 1 : -1;
        if (file) {
            fclose(file);
        }
    }
    return use_reg_debug > 0;
}

UseContainerHook* use_reg_make(const char* name, const char* handler, double value, int mode) {
    UseContainerHook* hook = new UseContainerHook();
    hook->name = name;
    hook->handler = use_reg_escape(handler);
    hook->value = value;
    hook->mode = mode;
    return hook;
}

void use_reg_store(UseContainerHook* hook, const char* handler, double value, int mode) {
    hook->handler = use_reg_escape(handler);
    hook->value = value;
    hook->mode = mode;
}

// ʕ •ᴥ•ʔ✿ name and handler strings; optional value and mode. Pushes 1 ✿ ʕ •ᴥ•ʔ
int native_register_use_container_item(void* state) {
    if (!g_is_str || !g_to_str || !g_push_num || !g_is_str(state, 1) || !g_is_str(state, 2)) {
        return 0;
    }
    double value = 0;
    int mode = 0;
    if (g_is_num && g_to_num && g_is_num(state, 3)) {
        value = g_to_num(state, 3);
    }
    if (g_is_num && g_to_num && g_is_num(state, 4)) {
        mode = use_reg_mode(g_to_num(state, 4));
    }
    const char* name = g_to_str(state, 1, nullptr);
    const char* handler = g_to_str(state, 2, nullptr);
    if (!name || !name[0] || !handler || !handler[0]) {
        return 0;
    }
    if (use_container_dispatching != 0) {
        use_container_pending.push_back(use_reg_make(name, handler, value, mode));
        g_push_num(state, 1.0);
        return 1;
    }
    if (UseContainerHook* found = use_reg_find(name)) {
        use_reg_store(found, handler, value, mode);
        use_reg_sort();
        g_push_num(state, 1.0);
        return 1;
    }
    use_container_hooks.push_back(use_reg_make(name, handler, value, mode));
    use_reg_sort();
    if (use_container_hooks.size() >= 50 && use_reg_debug_on()) {
        log_line("Registered container item hook %d: %s", static_cast<int>(use_container_hooks.size()), name);
    }
    g_push_num(state, 1.0);
    return 1;
}
