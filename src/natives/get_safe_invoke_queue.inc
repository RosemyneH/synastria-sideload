using SafeQueueTableFn = void (*)(void*, int, int);
using SafeQueueRawFn = void (*)(void*, int);
using SafeQueueCheckFn = int (*)(void*, int);

// ʕ •ᴥ•ʔ✿ WoWExt vector<string> begin/end at 0x100aed68. native_safe_invoke runs _G[name] immediately, so it does not push here. A later SynastriaSafeInvoke can append the global name and this getter will return it. ✿ ʕ •ᴥ•ʔ
std::vector<std::string> safe_invoke_queue;

SafeQueueTableFn safe_queue_table = nullptr;
SafeQueueRawFn safe_queue_rawset = nullptr;
SafeQueueCheckFn safe_queue_check = nullptr;
bool safe_queue_bound = false;

void safe_queue_bind() {
    if (safe_queue_bound) {
        return;
    }
    safe_queue_bound = true;
    void* found = nullptr;
    if (syn_resolve("lua_createtable", &found)) {
        safe_queue_table = reinterpret_cast<SafeQueueTableFn>(found);
    }
    found = nullptr;
    if (syn_resolve("lua_rawset", &found)) {
        safe_queue_rawset = reinterpret_cast<SafeQueueRawFn>(found);
    }
    found = nullptr;
    if (syn_resolve("lua_checkstack", &found)) {
        safe_queue_check = reinterpret_cast<SafeQueueCheckFn>(found);
    }
}

// ʕ •ᴥ•ʔ✿ no args; empty queue returns 0; otherwise one 1-based table of names, then the queue is cleared ✿ ʕ •ᴥ•ʔ
int native_safe_invoke_queue(void* state) {
    if (safe_invoke_queue.empty()) {
        return 0;
    }
    safe_queue_bind();
    if (!safe_queue_table || !safe_queue_rawset || !g_push_num || !g_push_str) {
        return 0;
    }
    safe_queue_table(state, 0, 0);
    int index = 1;
    for (const std::string& name : safe_invoke_queue) {
        if (safe_queue_check) {
            safe_queue_check(state, 2);
        }
        g_push_num(state, static_cast<double>(index));
        g_push_str(state, name.c_str());
        safe_queue_rawset(state, -3);
        ++index;
    }
    safe_invoke_queue.clear();
    return 1;
}
