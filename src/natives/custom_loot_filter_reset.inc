// ʕ •ᴥ•ʔ✿ 2 is reset on the shared dirty byte. Rules and options stay as they are. ✿ ʕ •ᴥ•ʔ
int native_loot_filter_reset(void* state) {
    (void)state;
    loot_filter_dirty = 2;
    return 0;
}
