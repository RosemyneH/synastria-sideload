// ʕ •ᴥ•ʔ✿ blob is 0x81, bytes +0x20..+0x23, a u16 name length, the name bytes, then count raw condition dwords. Base64 skips space, CR, and LF. '=' or a short tail pads with NUL, and NUL maps to 0xff, so that tail writes min(n, 3) bytes. ✿ ʕ •ᴥ•ʔ

const uint8_t* loot_import_rule_b64_table() {
    static uint8_t table[128];
    static int ready = 0;
    if (!ready) {
        std::memset(table, 0xff, sizeof(table));
        const char alpha[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        for (int i = 0; i < 64; ++i) {
            table[static_cast<uint8_t>(alpha[i])] = static_cast<uint8_t>(i);
        }
        ready = 1;
    }
    return table;
}

bool loot_import_rule_b64_ok(uint8_t ch) {
    if (ch >= 'A' && ch <= 'Z') {
        return true;
    }
    if (ch >= 'a' && ch <= 'z') {
        return true;
    }
    if (ch >= '0' && ch <= '9') {
        return true;
    }
    return ch == '+' || ch == '/';
}

void loot_import_rule_b64_pack(const uint8_t* table, const uint8_t chars[4], uint8_t out[3]) {
    uint8_t c0 = table[chars[0]];
    uint8_t c1 = table[chars[1]];
    uint8_t c2 = table[chars[2]];
    uint8_t c3 = table[chars[3]];
    out[0] = static_cast<uint8_t>((c0 << 2) + ((c1 >> 4) & 3));
    out[1] = static_cast<uint8_t>((c1 << 4) + ((c2 >> 2) & 0xf));
    out[2] = static_cast<uint8_t>((c2 << 6) + c3);
}

bool loot_import_rule_b64_emit(uint8_t* dst, int32_t cap, int32_t* pos, const uint8_t* bytes, int n) {
    if (*pos + n > cap) {
        return false;
    }
    if (n > 0) {
        std::memcpy(dst + *pos, bytes, static_cast<size_t>(n));
    }
    *pos += n;
    return true;
}

bool loot_import_rule_b64(const char* text, int32_t len, uint8_t* dst, int32_t cap, int32_t* out_len) {
    const uint8_t* table = loot_import_rule_b64_table();
    uint8_t chars[4] = {};
    int filled = 0;
    int32_t pos = 0;
    int32_t left = len;
    const uint8_t* cur = reinterpret_cast<const uint8_t*>(text);
    while (left != 0) {
        --left;
        uint8_t ch = *cur++;
        if (ch == ' ' || ch == '\r' || ch == '\n') {
            continue;
        }
        if (ch == '=') {
            break;
        }
        if (!loot_import_rule_b64_ok(ch)) {
            return false;
        }
        chars[filled++] = ch;
        if (filled < 4) {
            continue;
        }
        uint8_t packed[3];
        loot_import_rule_b64_pack(table, chars, packed);
        if (!loot_import_rule_b64_emit(dst, cap, &pos, packed, 3)) {
            return false;
        }
        filled = 0;
    }
    if (filled != 0) {
        for (int i = filled; i < 4; ++i) {
            chars[i] = 0;
        }
        uint8_t packed[3];
        loot_import_rule_b64_pack(table, chars, packed);
        int n = filled < 3 ? filled : 3;
        if (!loot_import_rule_b64_emit(dst, cap, &pos, packed, n)) {
            return false;
        }
    }
    *out_len = pos;
    return true;
}

bool loot_import_rule_take(const uint8_t* data, int32_t size, int32_t* pos, void* dst, int32_t n) {
    if (n < 0 || *pos > size - n) {
        return false;
    }
    if (n > 0 && dst) {
        std::memcpy(dst, data + *pos, static_cast<size_t>(n));
    }
    *pos += n;
    return true;
}

bool loot_import_rule_name(uint8_t* rule, const char* src) {
    size_t len = 0;
    while (src[len] != 0) {
        ++len;
    }
    uint8_t* text = rule + 0x24;
    uint32_t cap = 0;
    std::memcpy(&cap, text + 0x14, sizeof(cap));
    if (len <= cap) {
        char* dest = reinterpret_cast<char*>(text);
        if (cap >= 16u) {
            void* ptr = nullptr;
            std::memcpy(&ptr, text, sizeof(ptr));
            if (!ptr) {
                return false;
            }
            dest = static_cast<char*>(ptr);
        }
        if (len != 0) {
            std::memcpy(dest, src, len);
        }
        dest[len] = 0;
        uint32_t stored = static_cast<uint32_t>(len);
        std::memcpy(text + 0x10, &stored, sizeof(stored));
        return true;
    }
    if (len > 0x7fffffffu) {
        return false;
    }
    uint32_t grown = static_cast<uint32_t>(len) | 0x0fu;
    if (grown > 0x7fffffffu) {
        grown = 0x7fffffffu;
    } else {
        uint32_t half = cap >> 1;
        if (cap > 0x7fffffffu - half) {
            grown = 0x7fffffffu;
        } else {
            uint32_t sum = cap + half;
            if (grown < sum) {
                grown = sum;
            }
        }
    }
    char* buf = static_cast<char*>(std::malloc(static_cast<size_t>(grown) + 1u));
    if (!buf) {
        return false;
    }
    std::memcpy(buf, src, len);
    buf[len] = 0;
    if (cap >= 16u) {
        void* old = nullptr;
        std::memcpy(&old, text, sizeof(old));
        std::free(old);
    }
    void* stored_ptr = buf;
    std::memcpy(text, &stored_ptr, sizeof(stored_ptr));
    uint32_t stored = static_cast<uint32_t>(len);
    std::memcpy(text + 0x10, &stored, sizeof(stored));
    std::memcpy(text + 0x14, &grown, sizeof(grown));
    return true;
}

bool loot_import_rule_parse(const uint8_t* data, int32_t size, uint8_t hdr[4], uint32_t words[8], char name[33]) {
    int32_t pos = 0;
    uint8_t version = 0;
    if (!loot_import_rule_take(data, size, &pos, &version, 1)) {
        version = 0;
    }
    uint8_t low = static_cast<uint8_t>(version & 0x7f);
    if ((version >> 7) == 0 || low == 0 || low > 1) {
        return false;
    }
    if (!loot_import_rule_take(data, size, &pos, hdr, 4)) {
        return false;
    }
    uint16_t nlen = 0;
    if (!loot_import_rule_take(data, size, &pos, &nlen, 2)) {
        return false;
    }
    name[0] = 0;
    if (nlen != 0) {
        if (!loot_import_rule_take(data, size, &pos, nullptr, nlen)) {
            return false;
        }
        const uint8_t* bytes = data + pos - nlen;
        uint32_t slen = 0;
        while (slen < nlen && bytes[slen] != 0) {
            ++slen;
        }
        if (slen > 32) {
            slen = 32;
        }
        std::memcpy(name, bytes, slen);
        name[slen] = 0;
    }
    if (hdr[1] > 8) {
        return false;
    }
    for (uint8_t i = 0; i < hdr[1]; ++i) {
        if (!loot_import_rule_take(data, size, &pos, &words[i], 4)) {
            return false;
        }
    }
    return true;
}

int native_loot_filter_import_rule(void* state) {
    if (!loot_filter || !g_is_num || !g_to_num || !g_is_str || !g_to_str || !g_push_num) {
        return 0;
    }
    if (!g_is_num(state, 1) || !g_is_str(state, 2)) {
        return 0;
    }
    int32_t index = loot_set_round(g_to_num(state, 1)) - 1;
    if (static_cast<uint32_t>(index) > 31u || index >= static_cast<int32_t>(loot_filter->rule_count)) {
        return 0;
    }
    const char* text = g_to_str(state, 2, nullptr);
    if (!text) {
        return 0;
    }
    int32_t len = 0;
    while (text[len] != 0) {
        ++len;
    }
    uint8_t* raw = static_cast<uint8_t*>(std::malloc(0x2000));
    if (!raw) {
        return 0;
    }
    int32_t raw_len = 0;
    if (!loot_import_rule_b64(text, len, raw, 0x2000, &raw_len)) {
        std::free(raw);
        return 0;
    }
    uint8_t hdr[4] = {};
    uint32_t words[8] = {};
    char name[33] = {};
    if (!loot_import_rule_parse(raw, raw_len, hdr, words, name)) {
        std::free(raw);
        return 0;
    }
    std::free(raw);
    uint8_t* rule = loot_filter->rules[index];
    if (!loot_import_rule_name(rule, name)) {
        return 0;
    }
    // ʕ •ᴥ•ʔ✿ unread condition slots are stack garbage in the dll; stored as zero ✿ ʕ •ᴥ•ʔ
    std::memset(rule, 0, 0x20);
    for (uint8_t i = 0; i < hdr[1]; ++i) {
        std::memcpy(rule + static_cast<size_t>(i) * 4, &words[i], 4);
    }
    std::memcpy(rule + 0x20, hdr, 4);
    g_push_num(state, 1.0);
    return 1;
}
