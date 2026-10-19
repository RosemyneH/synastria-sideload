const char* zone_name_suffix(int diff) {
    static const char* zone_name_suffixes[] = {
        " (Normal)", " (Heroic)", " 10", " 25", " 10N", " 25N", " 10H", " 25H", " (Mythic)",
    };
    if ((diff & 0xff) == 0) {
        return nullptr;
    }
    int index = (diff & 0xf) - 1;
    if (index < 0 || index >= 9) {
        return nullptr;
    }
    return zone_name_suffixes[index];
}

const uint8_t* zone_name_db(const char* symbol) {
    void* found = nullptr;
    if (!syn_resolve(symbol, &found)) {
        return nullptr;
    }
    return static_cast<const uint8_t*>(found);
}

const char* zone_name_row(const uint8_t* table, int id, int name_off) {
    if (!table || id < 0) {
        return nullptr;
    }
    int32_t max_id = 0;
    int32_t min_id = 0;
    memcpy(&max_id, table + 0x10, 4);
    memcpy(&min_id, table + 0x14, 4);
    if (id < min_id || id > max_id) {
        return nullptr;
    }
    void** by_id = nullptr;
    memcpy(&by_id, table + 0x30, sizeof(by_id));
    if (!by_id) {
        return nullptr;
    }
    const uint8_t* row = static_cast<const uint8_t*>(by_id[id - min_id]);
    if (!row) {
        return nullptr;
    }
    const char* name = nullptr;
    memcpy(&name, row + name_off, sizeof(name));
    if (!name || name[0] == 0) {
        return nullptr;
    }
    return name;
}

// ʕ •ᴥ•ʔ✿ bit15: map id in 0x3ff, difficulty in bits 10-14; else zone id in 0x3fff, bit14 appends ", ..." ✿ ʕ •ᴥ•ʔ
std::string zone_name_text(int id) {
    if (static_cast<uint16_t>(id) == 0xffff) {
        return "?";
    }
    if (id & 0x8000) {
        int body = id & 0x7fff;
        const char* name = zone_name_row(zone_name_db("g_mapDB"), body & 0x3ff, 32);
        if (!name) {
            return "<Unknown Map>";
        }
        std::string text = name;
        if (const char* suffix = zone_name_suffix(body >> 10)) {
            text += suffix;
        }
        return text;
    }
    const char* name = zone_name_row(zone_name_db("g_areaTableDB"), id & 0x3fff, 48);
    if (!name) {
        return "<Unknown Zone>";
    }
    std::string text = name;
    if (id & 0x4000) {
        text += ", ...";
    }
    return text;
}

int native_get_zone_name(void* state) {
    if (!g_is_num || !g_to_num || !g_is_num(state, 1)) {
        return 0;
    }
    double raw = g_to_num(state, 1);
    if (!(raw >= 0.0 && raw < 65536.0)) {
        return 0;
    }
    std::string text = zone_name_text(static_cast<int>(raw));
    if (text.empty()) {
        return 0;
    }
    g_push_str(state, text.c_str());
    return 1;
}
