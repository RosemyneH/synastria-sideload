struct QuestDataRow {
    uint8_t faction = 0;
    uint32_t bits = 0;
    uint16_t at4 = 0;
    uint16_t at6 = 0;
    int16_t at8 = 0;
    uint8_t ata = 0;
    uint8_t atb = 0;
    uint16_t atc = 0;
    uint16_t ate = 0;
    uint16_t at10 = 0;
    uint16_t at12 = 0;
    uint16_t at14 = 0;
    int32_t at18 = 0;
    int32_t at1c = 0;
    int32_t at20 = 0;
    bool on = false;
};

std::vector<QuestDataRow> quest_data_rows;

struct QuestDataIn {
    const uint8_t* data;
    size_t size;
    size_t off = 0;
    bool ok = true;

    bool take(void* dst, size_t n) {
        if (!ok || off + n > size) {
            ok = false;
            return false;
        }
        memcpy(dst, data + off, n);
        off += n;
        return true;
    }

    uint8_t u8() {
        uint8_t value = 0;
        take(&value, 1);
        return value;
    }

    uint16_t u16() {
        uint16_t value = 0;
        take(&value, 2);
        return value;
    }

    uint32_t u32() {
        uint32_t value = 0;
        take(&value, 4);
        return value;
    }

    void cstr() {
        while (ok) {
            if (u8() == 0) {
                return;
            }
        }
    }

    uint32_t leb() {
        uint32_t value = 0;
        uint8_t byte = 0;
        do {
            byte = u8();
            value = (value << 7) | (byte & 0x7f);
        } while (ok && (byte & 0x80) != 0);
        return value;
    }

    void words() {
        uint8_t count = u8();
        for (uint8_t i = 0; i < count; ++i) {
            u16();
        }
    }

    void scaled(uint16_t& word, int32_t& scaled_out) {
        uint32_t packed = u32();
        word = static_cast<uint16_t>(packed);
        int32_t mag = static_cast<int32_t>((packed >> 16) & 0x7fff);
        scaled_out = (packed & 0x80000000u) ? -mag * 10 : mag * 10;
    }
};

void quest_data_skip_named(QuestDataIn& in, bool extra) {
    in.cstr();
    uint16_t mark = in.u16();
    if (mark != 0xffff) {
        in.u8();
    }
    if (!extra) {
        return;
    }
    uint8_t mask = in.u8();
    for (int bit = 0; bit < 3; ++bit) {
        if (mask & (1 << bit)) {
            in.u16();
        }
    }
}

void quest_data_read_row(QuestDataIn& in, QuestDataRow& row) {
    row.bits = in.u32();
    uint32_t bits = row.bits;
    if (bits & 0x1) {
        row.at4 = in.u16();
    } else if (bits & 0x2) {
        row.at4 = 0x44d;
    } else if (bits & 0x4) {
        row.at4 = 0x2b2;
    }
    if (bits & 0x8) {
        row.at6 = in.u16();
    }
    if (bits & 0x10) {
        row.at8 = static_cast<int16_t>(in.u16());
    }
    if (bits & 0x20) {
        row.ata = in.u8();
    }
    if (bits & 0x40) {
        row.atb = in.u8();
    }
    if (bits & 0x80) {
        row.atc = in.u16();
        row.ate = in.u16();
    }
    if (bits & 0x100) {
        in.scaled(row.at10, row.at18);
    }
    if (bits & 0x200) {
        in.scaled(row.at12, row.at1c);
    }
    if (bits & 0x400) {
        in.scaled(row.at14, row.at20);
    }
    if (bits & 0x800) {
        in.words();
    }
    if (bits & 0x1000) {
        in.u16();
    }
    if (bits & 0x2000) {
        in.words();
    }
    if (bits & 0x100000) {
        in.words();
    }
    if (bits & 0x200000) {
        in.words();
    }
    if (bits & 0x400000) {
        in.words();
    }
    if (bits & 0x800000) {
        in.words();
    }
    row.on = in.ok;
}

void load_quest_data(const uint8_t* data, size_t size) {
    quest_data_rows.clear();
    if (!data || size < 8) {
        return;
    }
    QuestDataIn in{data, size};
    uint32_t version = in.u32();
    if (version - 1 > 4) {
        return;
    }
    in.u32();
    if (in.off + 0x2000 > in.size) {
        return;
    }
    const uint8_t* bits = in.data + in.off;
    in.off += 0x2000;
    for (int id = 0; id < 0x10000 && in.ok; ++id) {
        if ((bits[id >> 3] & (1 << (id & 7))) == 0) {
            continue;
        }
        quest_data_skip_named(in, version >= 2);
    }
    uint32_t creatures = in.u32();
    for (uint32_t i = 0; i < creatures && in.ok; ++i) {
        in.leb();
        quest_data_skip_named(in, false);
    }
    uint32_t count = in.u32();
    if (version >= 5) {
        in.u32();
    }
    quest_data_rows.assign(0x10000, {});
    uint32_t quest = 0;
    int kept = 0;
    for (uint32_t i = 0; i < count && in.ok; ++i) {
        quest += in.leb();
        in.cstr();
        in.u16();
        uint8_t faction = version >= 3 ? in.u8() : 0;
        if ((faction & 0x4) == 0) {
            continue;
        }
        QuestDataRow row;
        row.faction = faction;
        quest_data_read_row(in, row);
        if (!row.on) {
            break;
        }
        if (quest == 0 || quest >= 0x10000) {
            continue;
        }
        quest_data_rows[quest] = row;
        ++kept;
    }
    log_line("quest data version %u rows %d", version, kept);
}

int native_get_quest_data(void* state) {
    if (!g_is_num || !g_is_num(state, 1)) {
        return 0;
    }
    double number = g_to_num(state, 1);
    if (!(number > -2147483648.0 && number < 2147483647.0)) {
        return 0;
    }
    double shifted = number >= 0 ? number + 0.5 : number - 0.5;
    if (!(shifted > -2147483648.0 && shifted < 2147483647.0)) {
        return 0;
    }
    int id = static_cast<int>(shifted);
    if (id <= 0 || id >= 0x10000) {
        return 0;
    }
    if (static_cast<size_t>(id) >= quest_data_rows.size() || !quest_data_rows[static_cast<size_t>(id)].on) {
        return 0;
    }
    const QuestDataRow& row = quest_data_rows[static_cast<size_t>(id)];
    // ʕ •ᴥ•ʔ✿ faction, record bits, +4 +6 +a +b +8, zero, +c +e, three (word, *10) pairs ✿ ʕ •ᴥ•ʔ
    g_push_num(state, row.faction);
    g_push_num(state, row.bits);
    g_push_num(state, row.at4);
    g_push_num(state, row.at6);
    g_push_num(state, row.ata);
    g_push_num(state, row.atb);
    g_push_num(state, row.at8);
    g_push_num(state, 0);
    g_push_num(state, row.atc);
    g_push_num(state, row.ate);
    g_push_num(state, row.at10);
    g_push_num(state, row.at18);
    g_push_num(state, row.at12);
    g_push_num(state, row.at1c);
    g_push_num(state, row.at14);
    g_push_num(state, row.at20);
    return 16;
}
