struct item_select_rec {
    const char* name;
    int32_t id;
};

// ʕ •ᴥ•ʔ✿ item_select is the filtered list of item_select_rec*. Each row is name at +0 and int32 item id at +4. Count is the number of pointers. ✿ ʕ •ᴥ•ʔ
std::vector<item_select_rec*> item_select;

int native_get_item_select_count(void* state) {
    int32_t count = static_cast<int32_t>(item_select.size());
    g_push_num(state, static_cast<double>(count));
    return 1;
}
