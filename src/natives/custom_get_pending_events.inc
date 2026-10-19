struct PendingEvtItem {
    uint32_t arg;
    uint32_t id;
};

std::vector<PendingEvtItem> pending_evt_order;
std::set<uint64_t> pending_evt_seen;

using PendingTableFn = void (*)(void*, int, int);
using PendingRawFn = void (*)(void*, int);
using PendingCheckFn = int (*)(void*, int);

PendingTableFn pending_evt_table = nullptr;
PendingRawFn pending_evt_rawset = nullptr;
PendingCheckFn pending_evt_check = nullptr;
bool pending_evt_bound = false;

void pending_evt_bind() {
    if (pending_evt_bound) {
        return;
    }
    pending_evt_bound = true;
    void* found = nullptr;
    if (syn_resolve("lua_createtable", &found)) {
        pending_evt_table = reinterpret_cast<PendingTableFn>(found);
    }
    found = nullptr;
    if (syn_resolve("lua_rawset", &found)) {
        pending_evt_rawset = reinterpret_cast<PendingRawFn>(found);
    }
    found = nullptr;
    if (syn_resolve("lua_checkstack", &found)) {
        pending_evt_check = reinterpret_cast<PendingCheckFn>(found);
    }
}

uint64_t pending_evt_key(uint32_t arg, uint32_t id) {
    return (static_cast<uint64_t>(id) << 32) | arg;
}

void pending_evt_clear() {
    pending_evt_seen.clear();
    pending_evt_order.clear();
}

// ʕ •ᴥ•ʔ✿ empty order returns nothing; otherwise a 1-based table of (id<<32)|arg doubles, then the dedup set and order are cleared ✿ ʕ •ᴥ•ʔ
int native_get_pending_events(void* state) {
    if (pending_evt_order.empty()) {
        return 0;
    }
    pending_evt_bind();
    if (!pending_evt_table || !pending_evt_rawset || !g_push_num) {
        return 0;
    }
    pending_evt_table(state, 0, 0);
    int index = 1;
    for (const PendingEvtItem& item : pending_evt_order) {
        if (pending_evt_check) {
            pending_evt_check(state, 2);
        }
        g_push_num(state, static_cast<double>(index));
        g_push_num(state, static_cast<double>(pending_evt_key(item.arg, item.id)));
        pending_evt_rawset(state, -3);
        ++index;
    }
    pending_evt_clear();
    return 1;
}
