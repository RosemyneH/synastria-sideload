struct PendingEvtExItem {
    uint8_t id;
    std::vector<uint32_t> args;
};

std::vector<PendingEvtExItem> pending_evt_ex_order;

// ʕ •ᴥ•ʔ✿ empty Ex queue returns nothing; otherwise a 1-based run of id, arg count, then each arg, and only that queue is cleared ✿ ʕ •ᴥ•ʔ
int native_get_pending_events_ex(void* state) {
    if (pending_evt_ex_order.empty()) {
        return 0;
    }
    pending_evt_bind();
    if (!pending_evt_table || !pending_evt_rawset || !g_push_num) {
        return 0;
    }
    pending_evt_table(state, 0, 0);
    int index = 1;
    for (const PendingEvtExItem& item : pending_evt_ex_order) {
        if (pending_evt_check) {
            pending_evt_check(state, 2);
        }
        g_push_num(state, static_cast<double>(index));
        g_push_num(state, static_cast<double>(item.id));
        pending_evt_rawset(state, -3);
        ++index;
        if (pending_evt_check) {
            pending_evt_check(state, 2);
        }
        g_push_num(state, static_cast<double>(index));
        g_push_num(state, static_cast<double>(static_cast<uint32_t>(item.args.size())));
        pending_evt_rawset(state, -3);
        ++index;
        for (uint32_t arg : item.args) {
            if (pending_evt_check) {
                pending_evt_check(state, 2);
            }
            g_push_num(state, static_cast<double>(index));
            g_push_num(state, static_cast<double>(arg));
            pending_evt_rawset(state, -3);
            ++index;
        }
    }
    pending_evt_ex_order.clear();
    return 1;
}
