struct SrcAtObj {
    uint32_t id = 0;
    uint8_t type = 0;
    uint8_t spawn = 0;
    uint16_t zone = 0;
    bool name_known = true;
    std::string name;
};

struct SrcAtSrc {
    uint16_t slot = 0;
    uint16_t chance = 0;
    uint16_t perthousand = 0;
    uint8_t type = 0;
};

std::vector<SrcAtObj> src_at_objs;
std::vector<std::vector<SrcAtSrc>> src_at_items;
bool src_at_ready = false;

struct SrcAtReader {
    const uint8_t* p = nullptr;
    const uint8_t* end = nullptr;
    bool ok = true;

    bool take(size_t n) {
        if (!ok || p + n > end) {
            ok = false;
            return false;
        }
        return true;
    }

    uint8_t u8() {
        if (!take(1)) {
            return 0;
        }
        return *p++;
    }

    uint16_t u16() {
        if (!take(2)) {
            return 0;
        }
        uint16_t v;
        memcpy(&v, p, 2);
        p += 2;
        return v;
    }

    uint32_t u32() {
        if (!take(4)) {
            return 0;
        }
        uint32_t v;
        memcpy(&v, p, 4);
        p += 4;
        return v;
    }

    uint32_t varint() {
        uint32_t v = 0;
        for (int i = 0; i < 5; ++i) {
            uint8_t b = u8();
            v = (v << 7) | (b & 0x7f);
            if ((b & 0x80) == 0) {
                return v;
            }
        }
        return v;
    }

    std::string str() {
        std::string out;
        while (ok) {
            uint8_t b = u8();
            if (!ok || b == 0) {
                break;
            }
            out.push_back(static_cast<char>(b));
        }
        return out;
    }

    void skip(size_t n) {
        if (take(n)) {
            p += n;
        }
    }
};

void src_at_skip_extra(SrcAtReader& in) {
    uint32_t flags = in.u32();
    if (flags & 0x1) {
        in.u16();
    }
    if (flags & 0x8) {
        in.u16();
    }
    if (flags & 0x10) {
        in.u16();
    }
    if (flags & 0x20) {
        in.u8();
    }
    if (flags & 0x40) {
        in.u8();
    }
    if (flags & 0x80) {
        in.u16();
        in.u16();
    }
    if (flags & 0x100) {
        in.u32();
    }
    if (flags & 0x200) {
        in.u32();
    }
    if (flags & 0x400) {
        in.u32();
    }
    if (flags & 0x800) {
        uint8_t n = in.u8();
        for (uint8_t i = 0; i < n; ++i) {
            in.u16();
        }
    }
    if (flags & 0x1000) {
        in.u16();
    }
    if (flags & 0x2000) {
        uint8_t n = in.u8();
        for (uint8_t i = 0; i < n; ++i) {
            in.u16();
        }
    }
    if (flags & 0x100000) {
        uint8_t n = in.u8();
        for (uint8_t i = 0; i < n; ++i) {
            in.u16();
        }
    }
    if (flags & 0x200000) {
        uint8_t n = in.u8();
        for (uint8_t i = 0; i < n; ++i) {
            in.u16();
        }
    }
    if (flags & 0x400000) {
        uint8_t n = in.u8();
        for (uint8_t i = 0; i < n; ++i) {
            in.u16();
        }
    }
    if (flags & 0x800000) {
        uint8_t n = in.u8();
        for (uint8_t i = 0; i < n; ++i) {
            in.u16();
        }
    }
}

uint16_t src_at_add_obj(std::map<uint32_t, uint16_t>& keys, uint32_t key, uint32_t id, uint8_t type, uint8_t spawn, uint16_t zone, std::string name, bool name_known) {
    if (src_at_objs.size() >= 0x10000) {
        return 0;
    }
    SrcAtObj obj;
    obj.id = id;
    obj.type = type;
    obj.spawn = spawn;
    obj.zone = zone;
    obj.name_known = name_known;
    obj.name = std::move(name);
    src_at_objs.push_back(std::move(obj));
    uint16_t slot = static_cast<uint16_t>(src_at_objs.size() - 1);
    keys[key] = slot;
    return slot;
}

double src_at_chance(uint16_t word) {
    if (word == 0) {
        return 0;
    }
    if (word == 0xffff) {
        return 101.0;
    }
    uint64_t bits = 0x3f5934c67f9b2ce6ULL;
    double scale;
    memcpy(&scale, &bits, 8);
    return static_cast<double>(static_cast<float>(static_cast<double>(word) * scale));
}

double src_at_perthousand(uint16_t word) {
    if ((word & 0x8000) == 0) {
        if (word > 0x7ff0) {
            return 0;
        }
        return static_cast<double>(static_cast<float>(word));
    }
    double scaled = static_cast<double>(word & 0x7fff) / 3200.0;
    return static_cast<double>(static_cast<float>(scaled));
}

double src_at_spawn(uint8_t spawn) {
    if ((spawn & 0x80) == 0) {
        return static_cast<double>(static_cast<float>(spawn));
    }
    return static_cast<double>(static_cast<float>(spawn & 0x7f) * 0.01f);
}

// ʕ •ᴥ•ʔ✿ 0xFFFF is "?"; other ids use the same map and area names as Custom_GetZoneName. ✿ ʕ •ᴥ•ʔ
const char* src_at_zone_text(uint16_t zone) {
    if (zone == 0xffff) {
        return "?";
    }
    static std::string held;
    held = zone_name_text(zone);
    if (held == "<Unknown Zone>" || held == "<Unknown Map>") {
        return nullptr;
    }
    return held.c_str();
}

void load_itemloc_source_at(const uint8_t* data, size_t size) {
    src_at_objs.clear();
    src_at_items.clear();
    src_at_ready = false;
    src_at_objs.emplace_back();
    if (!data || size < 8) {
        return;
    }
    SrcAtReader in;
    in.p = data;
    in.end = data + size;
    uint32_t version = in.u32();
    if (version - 1 > 4) {
        return;
    }
    in.u32();
    if (!in.take(0x2000)) {
        return;
    }
    const uint8_t* bits = in.p;
    in.p += 0x2000;
    std::map<uint32_t, uint16_t> keys;
    for (uint32_t i = 0; i < 0x10000 && in.ok; ++i) {
        if ((bits[i >> 3] & (1u << (i & 7))) == 0) {
            continue;
        }
        std::string name = in.str();
        uint16_t zone = in.u16();
        uint8_t spawn = zone == 0xffff ? 0 : in.u8();
        if (version >= 2) {
            uint8_t flags = in.u8();
            for (int b = 0; b < 3; ++b) {
                if (flags & (1u << b)) {
                    in.u16();
                }
            }
        }
        src_at_add_obj(keys, i, i, 0, spawn, zone, std::move(name), true);
    }
    uint32_t count = in.u32();
    uint32_t acc = 0;
    for (uint32_t n = 0; n < count && in.ok; ++n) {
        acc += in.varint();
        std::string name = in.str();
        uint16_t zone = in.u16();
        uint8_t spawn = zone == 0xffff ? 0 : in.u8();
        src_at_add_obj(keys, acc | 0x01000000u, acc, 1, spawn, zone, std::move(name), true);
    }
    count = in.u32();
    if (version >= 5) {
        in.u32();
    }
    acc = 0;
    for (uint32_t n = 0; n < count && in.ok; ++n) {
        acc += in.varint();
        std::string name = in.str();
        uint16_t zone = in.u16();
        if (version >= 3) {
            uint8_t flags = in.u8();
            if (flags & 0x4) {
                src_at_skip_extra(in);
            }
        }
        src_at_add_obj(keys, acc | 0x02000000u, acc, 2, 0, zone, std::move(name), true);
    }
    if (!in.ok || !in.take(0x2000)) {
        src_at_objs.clear();
        return;
    }
    bits = in.p;
    in.p += 0x2000;
    src_at_items.assign(0x10000, {});
    static const uint8_t kRoll[22] = {
        0, 0, 1, 0, 1, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0,
    };
    static const uint32_t kPrefix[22] = {
        0, 0x01000000u, 0x02000000u, 0x03000000u, 0x04000000u, 0,
        0x03000000u, 0x05000000u, 0x01000000u, 0, 0x03000000u, 0x03000000u,
        0x06000000u, 0, 0, 0xffffffffu, 0, 0x01000000u, 0x03000000u, 0x15000000u, 0, 0x01000000u,
    };
    static const uint8_t kMake[22] = {
        0xff, 0xff, 0xff, 3, 0xff, 0xff, 3, 5, 0xff, 0xff, 3, 3,
        6, 0xff, 0xff, 0xff, 0xff, 0xff, 3, 0x15, 0xff, 0xff,
    };
    for (uint32_t item = 0; item < 0x10000 && in.ok; ++item) {
        if ((bits[item >> 3] & (1u << (item & 7))) == 0) {
            continue;
        }
        std::vector<SrcAtSrc>& vec = src_at_items[item];
        while (in.ok) {
            uint8_t type = in.u8();
            if (!in.ok || type == 0xff) {
                break;
            }
            uint32_t group = in.varint();
            acc = 0;
            bool rolls = type <= 0x15 && kRoll[type] == 0;
            for (uint32_t n = 0; n < group && in.ok; ++n) {
                acc += in.varint();
                uint16_t chance = 0xffff;
                uint16_t perthousand = 0x7fff;
                if (rolls) {
                    chance = in.u16();
                    perthousand = in.u16();
                }
                if (type > 0x15 || kPrefix[type] == 0xffffffffu) {
                    continue;
                }
                uint32_t key = acc | kPrefix[type];
                auto found = keys.find(key);
                uint16_t slot = 0;
                if (found == keys.end()) {
                    if (kMake[type] == 0xff) {
                        continue;
                    }
                    bool known = type != 7;
                    slot = src_at_add_obj(keys, key, acc, kMake[type], 0, 0xffff, std::string(), known);
                    if (slot == 0) {
                        continue;
                    }
                } else {
                    slot = found->second;
                }
                SrcAtSrc src;
                src.slot = slot;
                src.chance = chance;
                src.perthousand = perthousand;
                src.type = type;
                vec.push_back(src);
            }
        }
    }
    if (!in.ok) {
        src_at_objs.clear();
        src_at_items.clear();
        return;
    }
    src_at_ready = true;
    log_line("ItemLoc source-at objs %zu", src_at_objs.size());
}

int native_itemloc_source_at(void* state) {
    if (!src_at_ready || !g_is_num || !g_is_num(state, 1) || !g_is_num(state, 2)) {
        return 0;
    }
    int item = arg_int(state, 1);
    int index = arg_int(state, 2) - 1;
    if (item < 1 || item > 0xffff || index < 0 || static_cast<size_t>(item) >= src_at_items.size()) {
        return 0;
    }
    const std::vector<SrcAtSrc>& vec = src_at_items[static_cast<size_t>(item)];
    if (index >= static_cast<int>(vec.size())) {
        return 0;
    }
    const SrcAtSrc& src = vec[static_cast<size_t>(index)];
    if (src.slot == 0 || src.slot >= src_at_objs.size()) {
        return 0;
    }
    const SrcAtObj& obj = src_at_objs[src.slot];
    uint8_t shown = src.type;
    if (shown == 0x14) {
        shown = 0;
    } else if (shown == 0x15) {
        shown = 1;
    }
    g_push_num(state, shown);
    g_push_num(state, obj.type);
    g_push_num(state, static_cast<double>(obj.id));
    g_push_num(state, src_at_chance(src.chance));
    g_push_num(state, src_at_perthousand(src.perthousand));
    const char* name = nullptr;
    bool name_nil = false;
    const char* zone = nullptr;
    bool zone_nil = false;
    uint16_t zid = 0;
    if (shown == 3 || shown == 6 || shown == 0x0a || shown == 0x0b || shown == 0x12) {
        name = "?";
        zone = "?";
        if (obj.id != 0 && obj.id < 0x10000) {
            const std::vector<SrcAtSrc>& nested = src_at_items[obj.id];
            if (!nested.empty() && nested[0].slot != 0 && nested[0].slot < src_at_objs.size()) {
                zid = src_at_objs[nested[0].slot].zone;
                zone = src_at_zone_text(zid);
                zone_nil = zone == nullptr;
            }
        }
    } else if (shown == 7) {
        name_nil = !obj.name_known;
        name = obj.name.c_str();
        zid = obj.zone;
        zone = src_at_zone_text(zid);
        zone_nil = zone == nullptr;
    } else if (shown == 0x0c) {
        name = "Disenchanting";
        zone = "";
    } else if (shown == 0x13) {
        name = "Player";
        zone = "";
    } else {
        name = obj.name.c_str();
        zid = obj.zone;
        zone = src_at_zone_text(zid);
        zone_nil = zone == nullptr;
    }
    if (name_nil) {
        g_push_nil(state);
    } else {
        g_push_str(state, name);
    }
    if (zone_nil) {
        g_push_nil(state);
    } else {
        g_push_str(state, zone);
    }
    g_push_num(state, src_at_spawn(obj.spawn));
    g_push_num(state, zid);
    return 9;
}
