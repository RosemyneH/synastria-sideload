struct fixup_rec {
    uint16_t word[3] = {};
};

std::map<uint32_t, fixup_rec> fixup_rows;

// ʕ •ᴥ•ʔ✿ Same clamp as the other ItemLoc natives: round half away from zero, then saturate ✿ ʕ •ᴥ•ʔ
int fixup_to_int(double value) {
    constexpr double k_min = -2147483648.0;
    constexpr double k_max = 2147483647.0;
    if (!(value > k_min)) {
        return static_cast<int>(0x80000000u);
    }
    if (!(value < k_max)) {
        return 0x7fffffff;
    }
    double rounded = value + (value >= 0.0 ? 0.5 : -0.5);
    if (!(rounded > k_min)) {
        return static_cast<int>(0x80000000u);
    }
    if (!(rounded < k_max)) {
        return 0x7fffffff;
    }
    return static_cast<int>(rounded);
}

void fixup_cstr(BinReader& in) {
    while (in.ok && in.u8() != 0) {
    }
}

bool fixup_bit(const uint8_t* bits, int id) {
    return (bits[id >> 3] >> (id & 7)) & 1;
}

void load_itemloc_fixup_obj_id(const uint8_t* data, size_t size) {
    fixup_rows.clear();
    if (!data || size < 8) {
        log_line("WoWExtItemLoc.bin fixup rejected");
        return;
    }
    BinReader in{data, size, 0, true};
    uint32_t version = in.u32();
    if (!in.ok || version < 1 || version > 5) {
        log_line("WoWExtItemLoc.bin fixup version %u rejected", version);
        return;
    }
    in.u32();
    uint8_t bits[0x2000];
    if (!in.take(bits, sizeof(bits))) {
        log_line("WoWExtItemLoc.bin fixup truncated at %zu", in.off);
        return;
    }

    std::map<uint32_t, fixup_rec> built;
    for (int id = 0; id < 0x10000 && in.ok; ++id) {
        if (!fixup_bit(bits, id)) {
            continue;
        }
        fixup_cstr(in);
        if (in.u16() != 0xffff) {
            in.u8();
        }
        if (version < 2) {
            continue;
        }
        uint8_t mask = in.u8();
        if (!mask) {
            continue;
        }
        fixup_rec rec;
        bool any = false;
        for (int bit = 0; bit < 3; ++bit) {
            if (mask & (1u << bit)) {
                rec.word[bit] = in.u16();
                any = any || rec.word[bit] != 0;
            }
        }
        if (in.ok && any) {
            built.emplace(static_cast<uint32_t>(id), rec);
        }
    }
    if (!in.ok) {
        log_line("WoWExtItemLoc.bin fixup truncated at %zu", in.off);
        return;
    }
    fixup_rows.swap(built);
    log_line("WoWExtItemLoc.bin fixup version %u ids %zu", version, fixup_rows.size());
}

// ʕ •ᴥ•ʔ✿ GetCustomGameData(12, 11), rounded. Only 1..3 index the three creature words. ✿ ʕ •ᴥ•ʔ
int fixup_mode_slot() {
    double value = game_num(12, 11);
    if (!(value > -2147483648.0) || !(value < 2147483648.0)) {
        return 0;
    }
    double rounded = value >= 0.0 ? value + 0.5 : value - 0.5;
    if (!(rounded > -2147483648.0) || !(rounded < 2147483648.0)) {
        return 0;
    }
    int slot = static_cast<int>(rounded);
    if (slot < 1 || slot > 3) {
        return 0;
    }
    return slot;
}

// ʕ •ᴥ•ʔ✿ Slot 3 falls back to word 1. Slot 2 and slot 1 do not fall back. A zero word keeps the caller id. ✿ ʕ •ᴥ•ʔ
int native_itemloc_fixup_obj_id(void* state) {
    if (!g_is_num || !g_to_num || !g_push_num || !g_is_num(state, 1) || !g_is_num(state, 2)) {
        return 0;
    }
    int type = fixup_to_int(g_to_num(state, 1));
    int id = fixup_to_int(g_to_num(state, 2));
    if (type == 2) {
        return 0;
    }
    uint32_t key = (static_cast<uint32_t>(type) << 24) | static_cast<uint32_t>(id);
    int slot = fixup_mode_slot();
    if (slot >= 1 && slot <= 3) {
        auto found = fixup_rows.find(key);
        if (found != fixup_rows.end()) {
            const uint16_t* word = found->second.word;
            while (slot > 0) {
                uint16_t picked = word[slot - 1];
                if (picked != 0) {
                    id = picked;
                    break;
                }
                slot -= slot >= 2 ? 2 : 1;
            }
        }
    }
    g_push_num(state, static_cast<double>(id));
    return 1;
}
