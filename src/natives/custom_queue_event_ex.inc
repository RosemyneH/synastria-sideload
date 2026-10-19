// ʕ •ᴥ•ʔ✿ one id plus numbers at stack 2..98; appends the id and each saturated arg, returns nothing ✿ ʕ •ᴥ•ʔ
int native_queue_event_ex(void* state) {
    if (!g_is_num || !g_to_num || !g_is_num(state, 1)) {
        return 0;
    }
    int id = queue_event_arg(g_to_num(state, 1));
    if (id <= 0 || id > 0xff) {
        return 0;
    }
    PendingEvtExItem item;
    item.id = static_cast<uint8_t>(id);
    for (int index = 2; index < 0x63; ++index) {
        if (!g_is_num(state, index)) {
            break;
        }
        item.args.push_back(static_cast<uint32_t>(queue_event_arg(g_to_num(state, index))));
    }
    pending_evt_ex_order.push_back(item);
    return 0;
}
