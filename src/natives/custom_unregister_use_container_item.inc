struct UseContainerHook {
    std::string name;
    std::string handler;
    double value;
    int32_t mode;
};

// ʕ •ᴥ•ʔ✿ Register is not in the tree yet, so the shared list is defined here. Live hooks are DLL 0x100aeda0 (pointer vector, cap 0x100aeda8). Each record is name at +0, handler at +0x18, double at +0x30, int at +0x38. use_container_pending is 0x100aed94. use_container_dispatching is 0x100aed18; a non-zero value means the use callback is walking the live list. A pending record with an empty handler is an unregister. ✿ ʕ •ᴥ•ʔ
std::vector<UseContainerHook*> use_container_hooks;
std::vector<UseContainerHook*> use_container_pending;
int use_container_dispatching = 0;

bool use_unreg_same(const char* left, const char* right) {
    while (*left == *right) {
        if (*left == 0) {
            return true;
        }
        ++left;
        ++right;
    }
    return false;
}

// ʕ •ᴥ•ʔ✿ one string, the hook name. A missing, empty, or unmatched name pushes nothing. While dispatching, queue a name-only record and push nothing. Otherwise delete the first name match and push 1. ✿ ʕ •ᴥ•ʔ
int native_unregister_use_container_item(void* state) {
    if (!g_is_str || !g_to_str || !g_push_num || !g_is_str(state, 1)) {
        return 0;
    }
    const char* name = g_to_str(state, 1, nullptr);
    if (!name || name[0] == 0) {
        return 0;
    }
    if (use_container_dispatching != 0) {
        UseContainerHook* hook = new UseContainerHook();
        hook->name = name;
        use_container_pending.push_back(hook);
        return 0;
    }
    for (size_t i = 0; i < use_container_hooks.size(); ++i) {
        UseContainerHook* hook = use_container_hooks[i];
        if (!use_unreg_same(hook->name.c_str(), name)) {
            continue;
        }
        use_container_hooks.erase(use_container_hooks.begin() + static_cast<std::ptrdiff_t>(i));
        delete hook;
        g_push_num(state, 1.0);
        return 1;
    }
    return 0;
}
