// ʕ •ᴥ•ʔ✿ dressup model pointer (frame+0x2A0, then +0xB4) to facing float. A zero facing is absent. DLL hash map at 0x100aef58. ✿ ʕ •ᴥ•ʔ
std::map<void*, float> dressup_facing;

int native_clear_dressup_facing(void* state) {
    (void)state;
    dressup_facing.clear();
    return 0;
}
