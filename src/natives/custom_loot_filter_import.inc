// ʕ •ᴥ•ʔ✿ One string. Base64 (A–Z a–z 0–9 + /), spaces CR LF skipped, '=' ends the group. Blob: u8 version 1, u8 rule count, u16 options, then each rule is type, cond count, outcome, notify, u16 name length, bytes, then cond-count little-endian dwords. ✿ ʕ •ᴥ•ʔ

struct loot_import_buf {
    uint8_t* data;
    int32_t pos;
    int32_t end;
    uint8_t error;
};

bool loot_import_accepted(uint8_t c) {
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

void loot_import_quartet(const uint8_t* table, const uint8_t group[4], uint8_t out[3]) {
    uint8_t v0 = table[group[0]];
    uint8_t v1 = table[group[1]];
    uint8_t v2 = table[group[2]];
    uint8_t v3 = table[group[3]];
    out[0] = static_cast<uint8_t>((v0 << 2) + ((v1 >> 4) & 3));
    out[1] = static_cast<uint8_t>((v1 << 4) + ((v2 >> 2) & 0xf));
    out[2] = static_cast<uint8_t>((v2 << 6) + v3);
}

// ʕ •ᴥ•ʔ✿ '=' pads the quartet with zero bytes. Those look up as 0xff, and the decoder still emits one byte per stored character. ✿ ʕ •ᴥ•ʔ
bool loot_import_decode(uint8_t* out, int32_t cap, int32_t* written, const char* text, int32_t len) {
    uint8_t table[256];
    std::memset(table, 0xff, sizeof(table));
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
        if (!loot_import_accepted(c)) {
            return false;
        }
        group[filled++] = c;
        if (filled != 4) {
            continue;
        }
        uint8_t bytes[3];
        loot_import_quartet(table, group, bytes);
        if (pos + 3 > cap) {
            err = 1;
        } else {
            std::memcpy(out + pos, bytes, 3);
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
        loot_import_quartet(table, group, bytes);
        if (pos + emit > cap) {
            err = 1;
        } else if (emit > 0) {
            std::memcpy(out + pos, bytes, static_cast<size_t>(emit));
            pos += emit;
        }
    }
    *written = pos;
    return err == 0;
}

bool loot_import_read(loot_import_buf* in, void* dst, int32_t n) {
    int32_t next = in->pos + n;
    if (next > in->end) {
        in->error = 1;
        return false;
    }
    if (in->data && dst) {
        std::memcpy(dst, in->data + in->pos, static_cast<size_t>(n));
    }
    in->pos = next;
    return in->data != nullptr;
}

uint32_t loot_import_size(const uint8_t* str) {
    uint32_t size = 0;
    std::memcpy(&size, str + 0x10, sizeof(size));
    return size;
}

uint32_t loot_import_cap(const uint8_t* str) {
    uint32_t cap = 0;
    std::memcpy(&cap, str + 0x14, sizeof(cap));
    return cap;
}

const char* loot_import_chars(const uint8_t* str) {
    if (loot_import_cap(str) < 16) {
        return reinterpret_cast<const char*>(str);
    }
    const char* ptr = nullptr;
    std::memcpy(&ptr, str, sizeof(ptr));
    return ptr;
}

void loot_import_assign(uint8_t* str, const char* src, uint32_t len) {
    uint32_t cap = loot_import_cap(str);
    if (len <= cap) {
        char* dst = cap >= 16 ? const_cast<char*>(loot_import_chars(str)) : reinterpret_cast<char*>(str);
        if (len) {
            std::memcpy(dst, src, len);
        }
        dst[len] = 0;
        std::memcpy(str + 0x10, &len, sizeof(len));
        return;
    }
    uint32_t grown = len | 0x0fu;
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
    // ʕ •ᴥ•ʔ✿ A zeroed rule has cap 0. Growth that still fits the 15-byte inline buffer stays inline so cap < 16 means the bytes are here. ✿ ʕ •ᴥ•ʔ
    if (grown < 16) {
        if (len) {
            std::memcpy(str, src, len);
        }
        reinterpret_cast<char*>(str)[len] = 0;
        uint32_t inline_cap = 0x0f;
        std::memcpy(str + 0x10, &len, sizeof(len));
        std::memcpy(str + 0x14, &inline_cap, sizeof(inline_cap));
        return;
    }
    char* buf = static_cast<char*>(std::malloc(static_cast<size_t>(grown) + 1));
    if (!buf) {
        return;
    }
    if (len) {
        std::memcpy(buf, src, len);
    }
    buf[len] = 0;
    if (cap >= 16) {
        std::free(const_cast<char*>(loot_import_chars(str)));
    }
    std::memcpy(str, &buf, sizeof(buf));
    std::memcpy(str + 0x10, &len, sizeof(len));
    std::memcpy(str + 0x14, &grown, sizeof(grown));
}

void loot_import_rule_init(uint8_t* rule) {
    std::memset(rule, 0, 0x3c);
    uint32_t cap = 0x0f;
    std::memcpy(rule + 0x38, &cap, sizeof(cap));
}

void loot_import_rule_free(uint8_t* rule) {
    if (loot_import_cap(rule + 0x24) < 16) {
        return;
    }
    std::free(const_cast<char*>(loot_import_chars(rule + 0x24)));
}

void loot_import_clear_rule(uint8_t* rule) {
    std::memset(rule + 0x20, 0, 4);
    if (loot_import_size(rule + 0x24) == 0) {
        return;
    }
    loot_import_assign(rule + 0x24, "", 0);
}

void loot_import_read_name(loot_import_buf* in, uint8_t* str) {
    uint16_t n16 = 0;
    if (!loot_import_read(in, &n16, 2) || n16 == 0) {
        loot_import_assign(str, "", 0);
        return;
    }
    int32_t n = n16;
    char* tmp = static_cast<char*>(std::malloc(static_cast<size_t>(n) + 1));
    bool copied = false;
    int32_t next = in->pos + n;
    if (next > in->end) {
        in->error = 1;
    } else {
        if (tmp && in->data) {
            std::memcpy(tmp, in->data + in->pos, static_cast<size_t>(n));
            copied = true;
        }
        in->pos = next;
    }
    if (!tmp || !copied) {
        loot_import_assign(str, "", 0);
        std::free(tmp);
        return;
    }
    tmp[n] = 0;
    loot_import_assign(str, tmp, static_cast<uint32_t>(std::strlen(tmp)));
    std::free(tmp);
}

bool loot_import_read_rule(uint8_t* rule, loot_import_buf* in, uint8_t flag) {
    if (flag == 0) {
        uint8_t got = 0;
        if (loot_import_read(in, &got, 1)) {
            flag = got;
        }
    }
    uint8_t low = static_cast<uint8_t>(flag & 0x7f);
    uint8_t high = static_cast<uint8_t>(flag >> 7);
    uint8_t nz = low == 0 ? 0 : 0xff;
    if ((high & nz) == 0 || low > 1) {
        return false;
    }
    uint8_t type = 0;
    uint8_t cond = 0;
    uint8_t outcome = 0;
    uint8_t notify = 0;
    if (loot_import_read(in, &type, 1)) {
        rule[0x20] = type;
    }
    if (loot_import_read(in, &cond, 1)) {
        rule[0x21] = cond;
    }
    if (loot_import_read(in, &outcome, 1)) {
        rule[0x22] = outcome;
    }
    if (loot_import_read(in, &notify, 1)) {
        rule[0x23] = notify;
    }
    loot_import_read_name(in, rule + 0x24);
    uint32_t size = loot_import_size(rule + 0x24);
    if (size > 0x20) {
        char keep[0x20];
        std::memcpy(keep, loot_import_chars(rule + 0x24), 0x20);
        loot_import_assign(rule + 0x24, keep, 0x20);
    }
    if (rule[0x21] > 8) {
        return false;
    }
    for (uint8_t i = 0; i < rule[0x21]; ++i) {
        uint32_t entry = 0;
        loot_import_read(in, &entry, 4);
        std::memcpy(rule + static_cast<int>(i) * 4, &entry, 4);
    }
    return in->error == 0;
}

bool loot_import_parse(loot_filter_obj* obj, loot_import_buf* in) {
    uint8_t version = 0;
    if (!loot_import_read(in, &version, 1) || version == 0 || version > 1) {
        return false;
    }
    uint8_t count = 0;
    if (!loot_import_read(in, &count, 1)) {
        return false;
    }
    uint16_t options = 0;
    if (!loot_import_read(in, &options, 2)) {
        return false;
    }
    if (count > 32) {
        return false;
    }
    obj->rule_count = count;
    obj->options = options;
    uint8_t flag = static_cast<uint8_t>(version | 0x80);
    for (uint8_t i = 0; i < count; ++i) {
        if (!loot_import_read_rule(obj->rules[i], in, flag)) {
            return false;
        }
    }
    if (count < 32) {
        for (int i = count; i < 32; ++i) {
            loot_import_clear_rule(obj->rules[i]);
        }
    }
    return in->error == 0;
}

void loot_import_commit(loot_filter_obj* dst, const loot_filter_obj* src) {
    for (int i = 0; i < 32; ++i) {
        const uint8_t* from = src->rules[i];
        uint8_t* to = dst->rules[i];
        std::memcpy(to, from, 0x24);
        const char* chars = loot_import_chars(from + 0x24);
        loot_import_assign(to + 0x24, chars, loot_import_size(from + 0x24));
    }
    dst->rule_count = src->rule_count;
    dst->options = src->options;
}

int native_loot_filter_import(void* state) {
    if (!loot_filter || !g_is_str || !g_to_str || !g_push_num || !g_is_str(state, 1)) {
        return 0;
    }
    const char* text = g_to_str(state, 1, nullptr);
    if (!text) {
        return 0;
    }
    uint8_t* raw = static_cast<uint8_t*>(std::malloc(0x2000));
    if (!raw) {
        return 0;
    }
    int32_t n = 0;
    if (!loot_import_decode(raw, 0x2000, &n, text, static_cast<int32_t>(std::strlen(text)))) {
        std::free(raw);
        return 0;
    }
    loot_filter_obj local{};
    for (int i = 0; i < 32; ++i) {
        loot_import_rule_init(local.rules[i]);
    }
    loot_import_buf buf{raw, 0, n, 0};
    bool ok = loot_import_parse(&local, &buf);
    if (ok) {
        loot_import_commit(loot_filter, &local);
        g_push_num(state, 1.0);
    }
    for (int i = 0; i < 32; ++i) {
        loot_import_rule_free(local.rules[i]);
    }
    std::free(raw);
    return ok ? 1 : 0;
}
