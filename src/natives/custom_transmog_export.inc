// ʕ •ᴥ•ʔ✿ mode 0 exports set 1..999, mode 1 imports one string. Wire is a little-endian u16 mask (bits 0..14 slots, bit 15 name), each present slot as u16, optional u8 length plus that many name bytes, a u8 enchant mask, then each present enchant as u16. Slots are category 0x1a at set*100+11, enchants at set*100+3, name is the category 0x1b string. Blob is standard base64; '=' pads with 0x00, which decodes as 0xff and still emits one byte per stored character. Import rewrites each slot to the lowest owned item with the same display. ✿ ʕ •ᴥ•ʔ

int tmog_export_int(double n) {
    constexpr double k_min = -2147483648.0;
    constexpr double k_max = 2147483647.0;
    if (!(n > k_min)) {
        return static_cast<int>(0x80000000u);
    }
    if (!(n < k_max)) {
        return 0x7fffffff;
    }
    double rounded = __builtin_round(n);
    if (!(rounded > k_min)) {
        return static_cast<int>(0x80000000u);
    }
    if (!(rounded < k_max)) {
        return 0x7fffffff;
    }
    return static_cast<int>(rounded);
}

uint16_t tmog_export_word(double n) {
    return static_cast<uint16_t>(static_cast<uint32_t>(static_cast<int>(n)));
}

bool tmog_export_tick(double value) {
    double shifted = value + 0.01;
    if (!(shifted > -2147483648.0) || !(shifted < 2147483647.0)) {
        return true;
    }
    return static_cast<int>(shifted) != 0;
}

bool tmog_export_owned(int id) {
    for (int forge = 0; forge <= 3; ++forge) {
        int key = (forge << 24) | id;
        if (game_num(11, key) >= 100.0) {
            return true;
        }
        if (tmog_export_tick(game_num(16, key)) || tmog_export_tick(game_num(17, key)) || tmog_export_tick(game_num(20, key))) {
            return true;
        }
    }
    return game_num(0x1c, id) > 0.5;
}

int tmog_export_top() {
    int id = 0xffff;
    while (id > 0) {
        if (item_row(id)) {
            return id;
        }
        --id;
    }
    return 0;
}

int tmog_export_fix(int id) {
    if (id <= 0 || id > 0xffff) {
        return id;
    }
    const ItemRow* row = item_row(id);
    if (!row || row->display == 0) {
        return id;
    }
    if (tmog_export_owned(id)) {
        return id;
    }
    uint32_t display = row->display;
    int top = tmog_export_top();
    for (int cand = 1; cand <= top; ++cand) {
        const ItemRow* other = item_row(cand);
        if (!other || other->display != display) {
            continue;
        }
        if (tmog_export_owned(cand)) {
            return cand;
        }
    }
    return id;
}

struct tmog_export_buf {
    uint8_t* data;
    int32_t pos;
    int32_t cap;
    uint8_t overflow;
};

void tmog_export_raw(tmog_export_buf* buf, const void* src, int32_t count) {
    int32_t next = buf->pos + count;
    if (count < 0 || next > buf->cap) {
        buf->overflow = 1;
        return;
    }
    if (count > 0 && buf->data) {
        memcpy(buf->data + buf->pos, src, static_cast<size_t>(count));
    }
    buf->pos = next;
}

bool tmog_export_alpha(uint8_t c) {
    if (c >= '0' && c <= '9') {
        return true;
    }
    if (c >= 'A' && c <= 'Z') {
        return true;
    }
    if (c >= 'a' && c <= 'z') {
        return true;
    }
    return c == '+' || c == '/';
}

void tmog_export_quartet(const uint8_t* table, const uint8_t group[4], uint8_t out[3]) {
    uint8_t v0 = table[group[0]];
    uint8_t v1 = table[group[1]];
    uint8_t v2 = table[group[2]];
    uint8_t v3 = table[group[3]];
    out[0] = static_cast<uint8_t>((v0 << 2) + ((v1 >> 4) & 3));
    out[1] = static_cast<uint8_t>((v1 << 4) + ((v2 >> 2) & 0xf));
    out[2] = static_cast<uint8_t>((v2 << 6) + v3);
}

bool tmog_export_b64(const uint8_t* src, int32_t len, uint8_t* dst, int32_t cap, int32_t* out_len) {
    static const char k_alpha[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    int32_t pos = 0;
    int32_t i = 0;
    while (i + 3 <= len) {
        uint8_t b0 = src[i];
        uint8_t b1 = src[i + 1];
        uint8_t b2 = src[i + 2];
        uint8_t sextet[4] = {
            static_cast<uint8_t>(b0 >> 2),
            static_cast<uint8_t>(((b0 & 3) << 4) | (b1 >> 4)),
            static_cast<uint8_t>(((b1 & 0xf) << 2) | (b2 >> 6)),
            static_cast<uint8_t>(b2 & 0x3f),
        };
        for (int k = 0; k < 4; ++k) {
            if (sextet[k] >= 64 || pos + 1 > cap) {
                return false;
            }
            dst[pos++] = static_cast<uint8_t>(k_alpha[sextet[k]]);
        }
        i += 3;
    }
    int32_t rem = len - i;
    if (rem > 0) {
        uint8_t b0 = src[i];
        uint8_t b1 = rem > 1 ? src[i + 1] : 0;
        uint8_t sextet[4] = {
            static_cast<uint8_t>(b0 >> 2),
            static_cast<uint8_t>(((b0 & 3) << 4) | (b1 >> 4)),
            static_cast<uint8_t>((b1 & 0xf) << 2),
            0,
        };
        int32_t data_chars = rem + 1;
        for (int32_t k = 0; k < data_chars; ++k) {
            if (sextet[k] >= 64 || pos + 1 > cap) {
                return false;
            }
            dst[pos++] = static_cast<uint8_t>(k_alpha[sextet[k]]);
        }
        for (int32_t k = 0; k < 3 - rem; ++k) {
            if (pos + 1 > cap) {
                return false;
            }
            dst[pos++] = '=';
        }
    }
    *out_len = pos;
    return true;
}

bool tmog_export_un64(uint8_t* out, int32_t cap, int32_t* written, const char* text, int32_t len) {
    uint8_t table[256];
    memset(table, 0xff, sizeof(table));
    const char* alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    for (int i = 0; i < 64; ++i) {
        table[static_cast<uint8_t>(alphabet[i])] = static_cast<uint8_t>(i);
    }
    int32_t pos = 0;
    uint8_t err = 0;
    uint8_t group[4] = {};
    int filled = 0;
    int32_t i = 0;
    while (i < len) {
        if (err) {
            break;
        }
        uint8_t c = static_cast<uint8_t>(text[i++]);
        if (c == ' ' || c == '\r' || c == '\n') {
            continue;
        }
        if (c == '=') {
            break;
        }
        if (!tmog_export_alpha(c)) {
            return false;
        }
        group[filled++] = c;
        if (filled != 4) {
            continue;
        }
        uint8_t bytes[3];
        tmog_export_quartet(table, group, bytes);
        if (pos + 3 > cap) {
            err = 1;
        } else {
            memcpy(out + pos, bytes, 3);
            pos += 3;
        }
        filled = 0;
    }
    if (filled > 0) {
        int emit = filled < 3 ? filled : 3;
        for (int k = filled; k < 4; ++k) {
            group[k] = 0;
        }
        uint8_t bytes[3];
        tmog_export_quartet(table, group, bytes);
        if (pos + emit > cap) {
            err = 1;
        } else if (emit > 0) {
            memcpy(out + pos, bytes, static_cast<size_t>(emit));
            pos += emit;
        }
    }
    *written = pos;
    return err == 0;
}

bool tmog_export_take(const uint8_t* src, int32_t len, int32_t* pos, void* dst, int32_t count, uint8_t* overflow) {
    int32_t next = *pos + count;
    if (count < 0 || next > len) {
        *overflow = 1;
        return false;
    }
    if (count > 0) {
        memcpy(dst, src + *pos, static_cast<size_t>(count));
    }
    *pos = next;
    return true;
}

std::string tmog_export_label(int set) {
    GameCat* cat = game_cat(0x1b);
    if (!cat) {
        return {};
    }
    auto found = cat->strs.find(set);
    if (found == cat->strs.end()) {
        return {};
    }
    return found->second;
}

int tmog_export_pack(int set, uint8_t* raw, int32_t cap, int32_t* out_len) {
    uint16_t slots[15] = {};
    uint16_t mask = 0;
    int base = set * 100;
    for (int i = 0; i < 15; ++i) {
        uint16_t word = tmog_export_word(game_num(0x1a, base + 0xb + i));
        slots[i] = word;
        if (word != 0) {
            mask = static_cast<uint16_t>(mask | static_cast<uint16_t>(1u << i));
        }
    }
    std::string name = tmog_export_label(set);
    uint32_t nbytes = static_cast<uint32_t>(name.size());
    if (nbytes != 0) {
        mask = static_cast<uint16_t>(mask | 0x8000u);
    }
    tmog_export_buf buf{raw, 0, cap, 0};
    tmog_export_raw(&buf, &mask, 2);
    for (int i = 0; i < 15; ++i) {
        if (slots[i] != 0) {
            tmog_export_raw(&buf, &slots[i], 2);
        }
    }
    if (nbytes != 0) {
        uint8_t stored = static_cast<uint8_t>(nbytes);
        tmog_export_raw(&buf, &stored, 1);
        tmog_export_raw(&buf, name.data(), static_cast<int32_t>(stored));
    }
    uint16_t enchants[4] = {};
    uint8_t emask = 0;
    for (int i = 0; i < 4; ++i) {
        uint16_t word = tmog_export_word(game_num(0x1a, base + 3 + i));
        enchants[i] = word;
        if (word != 0) {
            emask = static_cast<uint8_t>(emask | static_cast<uint8_t>(1u << i));
        }
    }
    tmog_export_raw(&buf, &emask, 1);
    for (int i = 0; i < 4; ++i) {
        if (enchants[i] != 0) {
            tmog_export_raw(&buf, &enchants[i], 2);
        }
    }
    if (buf.overflow) {
        return 0;
    }
    *out_len = buf.pos;
    return 1;
}

int tmog_export_apply(int set, const uint8_t* src, int32_t len) {
    int32_t pos = 0;
    uint8_t overflow = 0;
    uint16_t mask = 0;
    tmog_export_take(src, len, &pos, &mask, 2, &overflow);
    uint16_t slots[15] = {};
    for (int i = 0; i < 15; ++i) {
        if ((mask & static_cast<uint16_t>(1u << i)) == 0) {
            continue;
        }
        tmog_export_take(src, len, &pos, &slots[i], 2, &overflow);
    }
    std::string name;
    if (mask & 0x8000u) {
        uint8_t nlen = 0;
        if (tmog_export_take(src, len, &pos, &nlen, 1, &overflow) && nlen >= 0x24) {
            return 0;
        }
        if (!overflow && nlen > 0) {
            if (pos + nlen > len) {
                overflow = 1;
            } else {
                char tmp[0x28];
                memcpy(tmp, src + pos, nlen);
                tmp[nlen] = 0;
                pos += nlen;
                name = tmp;
            }
        }
    }
    uint8_t emask = 0;
    tmog_export_take(src, len, &pos, &emask, 1, &overflow);
    uint16_t enchants[4] = {};
    for (int i = 0; i < 4; ++i) {
        if ((emask & static_cast<uint8_t>(1u << i)) == 0) {
            continue;
        }
        tmog_export_take(src, len, &pos, &enchants[i], 2, &overflow);
    }
    if (overflow) {
        return 0;
    }
    int base = set * 100;
    for (int i = 0; i < 15; ++i) {
        int fixed = tmog_export_fix(slots[i]);
        set_num(0x1a, base + 0xb + i, static_cast<double>(fixed));
    }
    for (int i = 0; i < 4; ++i) {
        set_num(0x1a, base + 3 + i, static_cast<double>(enchants[i]));
    }
    g_game[0x1b].strs[set] = name;
    return 1;
}

int native_transmog_export(void* state) {
    if (!g_is_num || !g_to_num || !g_is_num(state, 1) || !g_is_num(state, 2)) {
        return 0;
    }
    int mode = tmog_export_int(g_to_num(state, 1));
    int set = tmog_export_int(g_to_num(state, 2));
    if (static_cast<uint32_t>(set) - 1u > 0x3e6u) {
        return 0;
    }
    if (mode == 0) {
        if (!g_push_str) {
            return 0;
        }
        uint8_t raw[0x80];
        uint8_t enc[0x100];
        int32_t raw_len = 0;
        int32_t enc_len = 0;
        if (!tmog_export_pack(set, raw, 0x80, &raw_len)) {
            return 0;
        }
        if (!tmog_export_b64(raw, raw_len, enc, 0x100, &enc_len)) {
            return 0;
        }
        std::string text(reinterpret_cast<const char*>(enc), static_cast<size_t>(enc_len));
        g_push_str(state, text.c_str());
        return 1;
    }
    if (mode != 1 || !g_is_str || !g_to_str || !g_push_num || !g_is_str(state, 3)) {
        return 0;
    }
    size_t slen = 0;
    const char* text = g_to_str(state, 3, &slen);
    if (!text) {
        return 0;
    }
    uint8_t raw[0x80];
    int32_t raw_len = 0;
    if (!tmog_export_un64(raw, 0x80, &raw_len, text, static_cast<int32_t>(slen))) {
        return 0;
    }
    if (!tmog_export_apply(set, raw, raw_len)) {
        return 0;
    }
    g_push_num(state, 1.0);
    return 1;
}
