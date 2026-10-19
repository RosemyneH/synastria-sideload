int32_t loot_export_rule_index(double value) {
    constexpr double k_min = -2147483648.0;
    constexpr double k_max = 2147483647.0;
    if (!(value > k_min)) {
        return 0x7fffffff;
    }
    if (!(value < k_max)) {
        return 0x7ffffffe;
    }
    double rounded = __builtin_round(value);
    if (!(rounded > k_min)) {
        return 0x7fffffff;
    }
    if (!(rounded < k_max)) {
        return 0x7ffffffe;
    }
    return static_cast<int32_t>(rounded) - 1;
}

struct loot_export_rule_buf {
    uint8_t* data;
    int32_t pos;
    int32_t cap;
    uint8_t overflow;
};

void loot_export_rule_put(loot_export_rule_buf* buf, const void* src, int32_t n) {
    int32_t next = buf->pos + n;
    if (next > buf->cap) {
        buf->overflow = 1;
        return;
    }
    if (buf->data && n > 0) {
        memcpy(buf->data + buf->pos, src, static_cast<size_t>(n));
    }
    buf->pos = next;
}

void loot_export_rule_put_name(loot_export_rule_buf* buf, const uint8_t* str) {
    int32_t len = 0;
    memcpy(&len, str + 0x10, sizeof(len));
    if (len > 0xffff) {
        len = 0xffff;
    }
    uint16_t word = static_cast<uint16_t>(len);
    loot_export_rule_put(buf, &word, 2);
    if (len <= 0) {
        return;
    }
    uint32_t cap = 0;
    memcpy(&cap, str + 0x14, sizeof(cap));
    const uint8_t* bytes = str;
    if (cap >= 16) {
        uint32_t ptr = 0;
        memcpy(&ptr, str, sizeof(ptr));
        bytes = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(ptr));
    }
    loot_export_rule_put(buf, bytes, len);
}

bool loot_export_rule_write(const uint8_t* rule, loot_export_rule_buf* buf) {
    uint8_t mark = 0x81;
    loot_export_rule_put(buf, &mark, 1);
    loot_export_rule_put(buf, rule + 0x20, 1);
    loot_export_rule_put(buf, rule + 0x21, 1);
    loot_export_rule_put(buf, rule + 0x22, 1);
    loot_export_rule_put(buf, rule + 0x23, 1);
    loot_export_rule_put_name(buf, rule + 0x24);
    uint8_t count = rule[0x21];
    for (uint8_t i = 0; i < count; ++i) {
        uint32_t word = 0;
        memcpy(&word, rule + static_cast<size_t>(i) * 4, sizeof(word));
        loot_export_rule_put(buf, &word, 4);
    }
    return buf->overflow == 0;
}

bool loot_export_rule_b64(const uint8_t* in, int32_t in_len, loot_export_rule_buf* out) {
    const char alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    uint8_t trio[3] = {};
    int filled = 0;
    for (int32_t i = 0; i < in_len; ++i) {
        if (out->overflow) {
            break;
        }
        trio[filled++] = in[i];
        if (filled < 3) {
            continue;
        }
        uint8_t sext[4] = {
            static_cast<uint8_t>(trio[0] >> 2),
            static_cast<uint8_t>(((trio[0] & 3) << 4) | (trio[1] >> 4)),
            static_cast<uint8_t>(((trio[1] & 0xf) << 2) | (trio[2] >> 6)),
            static_cast<uint8_t>(trio[2] & 0x3f),
        };
        for (int s = 0; s < 4; ++s) {
            loot_export_rule_put(out, &alphabet[sext[s]], 1);
        }
        filled = 0;
    }
    if (filled == 0) {
        return out->overflow == 0;
    }
    for (int n = filled; n < 3; ++n) {
        trio[n] = 0;
    }
    uint8_t sext[4] = {
        static_cast<uint8_t>(trio[0] >> 2),
        static_cast<uint8_t>(((trio[0] & 3) << 4) | (trio[1] >> 4)),
        static_cast<uint8_t>(((trio[1] & 0xf) << 2) | (trio[2] >> 6)),
        static_cast<uint8_t>(trio[2] & 0x3f),
    };
    for (int s = 0; s < filled + 1; ++s) {
        loot_export_rule_put(out, &alphabet[sext[s]], 1);
    }
    for (int pad = filled; pad < 3; ++pad) {
        char eq = '=';
        loot_export_rule_put(out, &eq, 1);
    }
    return out->overflow == 0;
}

// ʕ •ᴥ•ʔ✿ one 1-based rule index. Blob is 0x81, bytes +0x20..+0x23, a u16 name length, the name, then each condition dword. Result is standard base64. ✿ ʕ •ᴥ•ʔ
int native_loot_filter_export_rule(void* state) {
    if (!g_is_num || !g_to_num || !g_push_str) {
        return 0;
    }
    int32_t index = -1;
    if (g_is_num(state, 1)) {
        index = loot_export_rule_index(g_to_num(state, 1));
    }
    if (!loot_filter || index < 0 || index >= static_cast<int>(loot_filter->rule_count)) {
        return 0;
    }
    constexpr int32_t k_cap = 0x2000;
    uint8_t raw[k_cap];
    uint8_t text[k_cap];
    loot_export_rule_buf raw_buf{raw, 0, k_cap, 0};
    if (!loot_export_rule_write(loot_filter->rules[index], &raw_buf)) {
        return 0;
    }
    loot_export_rule_buf text_buf{text, 0, k_cap, 0};
    if (!loot_export_rule_b64(raw, raw_buf.pos, &text_buf)) {
        return 0;
    }
    std::string encoded(reinterpret_cast<char*>(text), static_cast<size_t>(text_buf.pos));
    g_push_str(state, encoded.c_str());
    return 1;
}
