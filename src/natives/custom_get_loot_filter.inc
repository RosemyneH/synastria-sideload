// ʕ •ᴥ•ʔ✿ loot_filter is the one shared object. Null until filled. 32 rules of 0x3c occupy +0x0 through +0x77f. +0x780 u8 rule_count, +0x782 u16 options. ✿ ʕ •ᴥ•ʔ
struct loot_filter_obj {
    uint8_t rules[32][0x3c];
    uint8_t rule_count;
    uint8_t pad_781;
    uint16_t options;
};

loot_filter_obj* loot_filter = nullptr;

int native_get_loot_filter(void* state) {
    if (!loot_filter) {
        return 0;
    }
    g_push_num(state, static_cast<double>(loot_filter->rule_count));
    g_push_num(state, static_cast<double>(loot_filter->options));
    return 2;
}
