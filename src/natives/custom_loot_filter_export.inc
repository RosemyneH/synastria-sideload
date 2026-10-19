// ʕ •ᴥ•ʔ✿ wire is 0x01, rule_count, options u16, then each rule: bytes +0x20..+0x23, msvc name at +0x24 as u16 length plus bytes, then entry_count raw dwords from the rule. that buffer is standard base64. ✿ ʕ •ᴥ•ʔ
struct loot_export_buf {
    uint8_t* data;
    int32_t pos;
    int32_t cap;
    uint8_t overflow;
};

void loot_export_raw(loot_export_buf* buf, const void* src, int32_t count) {
    if (count < 0 || buf->pos + count > buf->cap) {
        buf->overflow = 1;
        return;
    }
    if (count > 0) {
        std::memcpy(buf->data + buf->pos, src, static_cast<size_t>(count));
    }
    buf->pos += count;
}

void loot_export_name(loot_export_buf* buf, const uint8_t* text) {
    uint32_t size = 0;
    uint32_t cap = 0;
    std::memcpy(&size, text + 0x10, sizeof(size));
    std::memcpy(&cap, text + 0x14, sizeof(cap));
    uint32_t stored = size;
    if (static_cast<int32_t>(size) > 0xffff) {
        stored = 0xffff;
    }
    uint16_t len = static_cast<uint16_t>(stored);
    loot_export_raw(buf, &len, 2);
    if (static_cast<int32_t>(stored) <= 0) {
        return;
    }
    const uint8_t* chars = text;
    if (cap >= 16) {
        uint32_t ptr = 0;
        std::memcpy(&ptr, text, sizeof(ptr));
        if (ptr == 0) {
            buf->overflow = 1;
            return;
        }
        chars = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(ptr));
    }
    loot_export_raw(buf, chars, static_cast<int32_t>(stored));
}

void loot_export_rule(loot_export_buf* buf, const uint8_t* rule) {
    loot_export_raw(buf, rule + 0x20, 4);
    loot_export_name(buf, rule + 0x24);
    const uint8_t* obj = reinterpret_cast<const uint8_t*>(loot_filter);
    const uint8_t* end = obj + sizeof(*loot_filter);
    uint8_t entries = rule[0x21];
    for (uint8_t i = 0; i < entries; ++i) {
        const uint8_t* word = rule + static_cast<size_t>(i) * 4;
        if (word < obj || word + 4 > end) {
            buf->overflow = 1;
            return;
        }
        loot_export_raw(buf, word, 4);
    }
}

bool loot_export_pack(loot_export_buf* buf) {
    uint8_t version = 1;
    loot_export_raw(buf, &version, 1);
    loot_export_raw(buf, &loot_filter->rule_count, 1);
    loot_export_raw(buf, &loot_filter->options, 2);
    uint8_t count = loot_filter->rule_count;
    if (count > 32) {
        count = 32;
    }
    for (uint8_t i = 0; i < count; ++i) {
        loot_export_rule(buf, loot_filter->rules[i]);
    }
    return buf->overflow == 0;
}

bool loot_export_b64(const uint8_t* src, int32_t len, uint8_t* dst, int32_t cap, int32_t* out_len) {
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

int native_loot_filter_export(void* state) {
    if (!loot_filter || !g_push_str) {
        return 0;
    }
    uint8_t* raw = static_cast<uint8_t*>(std::malloc(0x2000));
    uint8_t* enc = static_cast<uint8_t*>(std::malloc(0x2000));
    if (!raw || !enc) {
        std::free(raw);
        std::free(enc);
        return 0;
    }
    loot_export_buf bin{raw, 0, 0x2000, 0};
    int result = 0;
    if (loot_export_pack(&bin)) {
        int32_t enc_len = 0;
        if (loot_export_b64(raw, bin.pos, enc, 0x2000, &enc_len)) {
            std::string text(reinterpret_cast<const char*>(enc), static_cast<size_t>(enc_len));
            g_push_str(state, text.c_str());
            result = 1;
        }
    }
    std::free(raw);
    std::free(enc);
    return result;
}
