// ʕ •ᴥ•ʔ✿ ItemLocGetSourcesCompacted: six results, or none when the filtered list is empty ✿ ʕ •ᴥ•ʔ

struct CompactObj {
    uint32_t id = 0;
    uint8_t type = 0;
    uint16_t zone = 0;
    uint16_t flags = 0;
    std::string name;
    std::vector<uint16_t> need_all;
    std::vector<uint16_t> need_any;
};

struct CompactSrc {
    uint16_t slot = 0;
    uint16_t chance = 0xffff;
    uint8_t type = 0;
};

struct CompactHit {
    int index = 0;
    std::vector<uint32_t> chain;
};

std::vector<CompactObj> compact_objs;
std::vector<std::vector<CompactSrc>> compact_items;
std::map<uint32_t, uint16_t> compact_keys;
bool compact_ready = false;

using CompactCreate = void (*)(void*, int, int);
using CompactRawSet = void (*)(void*, int);
using CompactCheck = int (*)(void*, int);

CompactCreate compact_create = nullptr;
CompactRawSet compact_rawset = nullptr;
CompactCheck compact_check = nullptr;
bool compact_lua = false;

const uint8_t compact_roll[0x16] = {
    0, 0, 1, 0, 1, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0,
};

const uint32_t compact_prefix[0x16] = {
    0, 0x01000000u, 0x02000000u, 0x03000000u, 0x04000000u, 0,
    0x03000000u, 0x05000000u, 0x01000000u, 0, 0x03000000u, 0x03000000u,
    0x06000000u, 0, 0, 0xffffffffu, 0, 0x01000000u, 0x03000000u, 0x15000000u, 0, 0x01000000u,
};

const uint8_t compact_make[0x16] = {
    0xff, 0xff, 0xff, 3, 0xff, 0xff, 3, 5, 0xff, 0xff, 3, 3,
    6, 0xff, 0xff, 0xff, 0xff, 0xff, 3, 0x15, 0xff, 0xff,
};

uint32_t compact_varint(BinReader& in) {
    uint32_t value = 0;
    for (int i = 0; i < 5 && in.ok; ++i) {
        uint8_t byte = in.u8();
        value = (value << 7) | (byte & 0x7fu);
        if ((byte & 0x80) == 0) {
            break;
        }
    }
    return value;
}

std::string compact_str(BinReader& in) {
    std::string out;
    while (in.ok) {
        uint8_t byte = in.u8();
        if (!in.ok || byte == 0) {
            break;
        }
        out.push_back(static_cast<char>(byte));
    }
    return out;
}

std::vector<uint16_t> compact_words(BinReader& in) {
    std::vector<uint16_t> words;
    uint8_t count = in.u8();
    words.reserve(count);
    for (uint8_t i = 0; i < count && in.ok; ++i) {
        words.push_back(in.u16());
    }
    return words;
}

void compact_skip_words(BinReader& in) {
    uint8_t count = in.u8();
    for (uint8_t i = 0; i < count && in.ok; ++i) {
        in.u16();
    }
}

void compact_extra(BinReader& in, CompactObj& obj) {
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
        compact_skip_words(in);
    }
    if (flags & 0x1000) {
        in.u16();
    }
    if (flags & 0x2000) {
        compact_skip_words(in);
    }
    if (flags & 0x100000) {
        compact_skip_words(in);
    }
    if (flags & 0x200000) {
        obj.need_all = compact_words(in);
    }
    if (flags & 0x400000) {
        obj.need_any = compact_words(in);
    }
    if (flags & 0x800000) {
        compact_skip_words(in);
    }
}

uint16_t compact_add_obj(uint32_t key, uint32_t id, uint8_t type, uint16_t zone, std::string name) {
    if (compact_objs.size() >= 0x10000) {
        return 0;
    }
    CompactObj obj;
    obj.id = id;
    obj.type = type;
    obj.zone = zone;
    obj.name = std::move(name);
    compact_objs.push_back(std::move(obj));
    uint16_t slot = static_cast<uint16_t>(compact_objs.size() - 1);
    compact_keys[key] = slot;
    return slot;
}

float compact_chance(uint16_t word) {
    if (word == 0) {
        return 0.f;
    }
    if (word == 0xffff) {
        return 100.f;
    }
    uint64_t bits = 0x3f5934c67f9b2ce6ULL;
    double scale;
    memcpy(&scale, &bits, 8);
    float value = static_cast<float>(static_cast<double>(word) * scale);
    if (value > 100.5f) {
        return 100.f;
    }
    return value;
}

uint16_t compact_zone_key(uint16_t zone, uint8_t type) {
    if (zone == 0) {
        return 0;
    }
    if ((type == 0x14 || type == 0x15) && static_cast<int16_t>(zone) < 0 && zone < 0xfff0) {
        zone = static_cast<uint16_t>((zone & 0xe7ff) | 0x2400);
    }
    return static_cast<uint16_t>(zone & 0xc3ff);
}

bool compact_spans_zones(int item) {
    if (item <= 0 || item >= 0x10000) {
        return false;
    }
    const std::vector<CompactSrc>& sources = compact_items[static_cast<size_t>(item)];
    uint16_t seen = 0;
    bool any = false;
    for (const CompactSrc& src : sources) {
        if (src.slot == 0 || src.slot >= compact_objs.size()) {
            continue;
        }
        uint16_t key = compact_zone_key(compact_objs[src.slot].zone, src.type);
        if (key == 0) {
            continue;
        }
        if (!any) {
            seen = key;
            any = true;
            continue;
        }
        if (seen != key) {
            return true;
        }
    }
    return false;
}

bool compact_on_log(int id) {
    load_quest_accept();
    quest_accept_bind();
    return quest_accept_on_log(id);
}

int compact_quest_status(const CompactObj& obj) {
    if ((obj.flags & 0xc) != 4) {
        return -4;
    }
    load_quest_accept();
    quest_accept_bind();
    return quest_accept_check(static_cast<int>(obj.id));
}

void compact_chain_list(const std::vector<uint16_t>& words, std::vector<uint32_t>& out) {
    for (uint16_t word : words) {
        uint32_t id = word & 0x7fffu;
        if (id != 0) {
            out.push_back(id);
        }
    }
}

void compact_chain(const CompactObj& obj, std::vector<uint32_t>& out) {
    compact_chain_list(obj.need_all, out);
    compact_chain_list(obj.need_any, out);
}

bool compact_chain_blocked(const std::vector<uint32_t>& chain) {
    if (chain.empty()) {
        return true;
    }
    for (uint32_t id : chain) {
        if (id == 0 || !compact_on_log(static_cast<int>(id))) {
            return false;
        }
    }
    return true;
}

int compact_mask(const CompactSrc& src, int item, int either) {
    int mask = 0;
    uint8_t type = src.type;
    if (type == 9) {
        mask |= 0x1;
    }
    if (type == 5 || type == 6) {
        mask |= 0x2;
    }
    if ((either & 4) && compact_spans_zones(item)) {
        mask |= 0x4;
    }
    if (type == 2) {
        mask |= 0x8;
    }
    if (type == 0 || type == 0x14) {
        mask |= 0x10;
    }
    if (type == 1 || type == 0x15) {
        mask |= 0x20;
    }
    if (type == 3) {
        mask |= 0x40;
    }
    if (type == 0x0e) {
        mask |= 0x80;
    }
    return mask;
}

bool compact_keep(int item, const CompactSrc& src, int white, int black, float min_chance, float max_chance, int other, int zone) {
    if (src.slot == 0 || src.slot >= compact_objs.size()) {
        return false;
    }
    const CompactObj& obj = compact_objs[src.slot];
    if (obj.type != 2) {
        uint16_t area = obj.zone;
        if ((src.type == 0x14 || src.type == 0x15) && static_cast<int16_t>(area) < 0 && area < 0xfff0) {
            area = static_cast<uint16_t>((area & 0xe7ff) | 0x2400);
        }
        if (!zone_contains_match(static_cast<uint32_t>(zone), area)) {
            return false;
        }
    }
    if (min_chance > 0.f || !(100.f <= max_chance)) {
        float chance = compact_chance(src.chance);
        if (min_chance > chance || chance > max_chance) {
            return false;
        }
    }
    if (item < 0 || item > 0xffff || !item_row(item)) {
        return false;
    }
    if (white != 0 || black != 0) {
        int mask = compact_mask(src, item, white | black);
        if (white != 0 && (white & mask) == 0) {
            return false;
        }
        if ((black & mask) != 0) {
            return false;
        }
    }
    int side = (other & 4) ? 2 : 1;
    if (obj.type == 2 && obj.id != 0) {
        if ((other & 1) && obj.id > 0 && compact_on_log(static_cast<int>(obj.id))) {
            return false;
        }
        if (other & 2) {
            int bits = obj.flags & 3;
            if (bits != 0 && (side & bits) == 0) {
                return false;
            }
        }
    }
    return true;
}

float compact_arg_float(void* state, int index, float fallback) {
    if (!g_is_num || !g_to_num || !g_is_num(state, index)) {
        return fallback;
    }
    return static_cast<float>(g_to_num(state, index));
}

void compact_bind_lua() {
    if (compact_lua) {
        return;
    }
    compact_lua = true;
    syn_resolve("lua_createtable", reinterpret_cast<void**>(&compact_create));
    if (!syn_resolve("lua_rawset", reinterpret_cast<void**>(&compact_rawset))) {
        syn_resolve("lua_settable", reinterpret_cast<void**>(&compact_rawset));
    }
    syn_resolve("lua_checkstack", reinterpret_cast<void**>(&compact_check));
}

void compact_push_num(void* state, int value) {
    if (value < 0) {
        g_push_num(state, -1.0);
        return;
    }
    g_push_num(state, static_cast<double>(value));
}

const char* compact_name(int src_type, int obj_type, int obj_id) {
    if (src_type < 0) {
        return "Various";
    }
    if (src_type == 3 || src_type == 6 || src_type == 0x0a || src_type == 0x0b || src_type == 0x12) {
        if (obj_id > 0 && obj_id < 0x10000) {
            const ItemRow* row = item_row(obj_id);
            if (row) {
                return row->name.c_str();
            }
        }
    } else if (src_type == 0x0c) {
        return "Disenchanting";
    } else if (src_type == 0x13) {
        return "Player";
    } else if (obj_type >= 0 && obj_id > 0) {
        uint32_t key = (static_cast<uint32_t>(obj_type) << 24) | static_cast<uint32_t>(obj_id);
        auto found = compact_keys.find(key);
        if (found != compact_keys.end() && found->second < compact_objs.size()) {
            return compact_objs[found->second].name.c_str();
        }
    }
    switch (src_type) {
    case 0:
        return "Various Enemies";
    case 1:
        return "Various Objects";
    case 2:
        return "Various Quests";
    case 3:
        return "Various Items";
    case 5:
    case 6:
        return "Various Crafting";
    case 7:
    case 8:
        return "Various Fishing";
    case 9:
        return "Various Vendors";
    case 0x0e:
        return "Various Pickpocket";
    default:
        return "Various";
    }
}

void load_itemloc_sources_compacted(const uint8_t* data, size_t size) {
    compact_objs.clear();
    compact_items.clear();
    compact_keys.clear();
    compact_ready = false;
    compact_objs.emplace_back();
    if (!data || size < 8) {
        return;
    }
    BinReader in{data, size, 0, true};
    uint32_t version = in.u32();
    in.u32();
    if (version < 1 || version > 5) {
        return;
    }
    if (in.off + 0x2000 > in.size) {
        return;
    }
    const uint8_t* bits = in.data + in.off;
    in.off += 0x2000;
    for (uint32_t id = 0; id < 0x10000 && in.ok; ++id) {
        if ((bits[id >> 3] & (1u << (id & 7))) == 0) {
            continue;
        }
        std::string name = compact_str(in);
        uint16_t zone = in.u16();
        if (zone != 0xffff) {
            in.u8();
        }
        if (version >= 2) {
            uint8_t flags = in.u8();
            for (int bit = 0; bit < 3; ++bit) {
                if (flags & (1 << bit)) {
                    in.u16();
                }
            }
        }
        compact_add_obj(id, id, 0, zone, std::move(name));
    }
    uint32_t count = in.u32();
    uint32_t acc = 0;
    for (uint32_t n = 0; n < count && in.ok; ++n) {
        acc += compact_varint(in);
        std::string name = compact_str(in);
        uint16_t zone = in.u16();
        if (zone != 0xffff) {
            in.u8();
        }
        compact_add_obj(acc | 0x01000000u, acc, 1, zone, std::move(name));
    }
    count = in.u32();
    if (version >= 5) {
        in.u32();
    }
    acc = 0;
    for (uint32_t n = 0; n < count && in.ok; ++n) {
        acc += compact_varint(in);
        std::string name = compact_str(in);
        uint16_t zone = in.u16();
        uint16_t flags = 0;
        CompactObj extra;
        if (version >= 3) {
            flags = in.u8();
            if (flags & 0x4) {
                compact_extra(in, extra);
            }
        }
        uint16_t slot = compact_add_obj(acc | 0x02000000u, acc, 2, zone, std::move(name));
        if (slot != 0 && slot < compact_objs.size()) {
            compact_objs[slot].flags = flags;
            compact_objs[slot].need_all = std::move(extra.need_all);
            compact_objs[slot].need_any = std::move(extra.need_any);
        }
    }
    if (!in.ok || in.off + 0x2000 > in.size) {
        compact_objs.clear();
        compact_keys.clear();
        return;
    }
    bits = in.data + in.off;
    in.off += 0x2000;
    compact_items.assign(0x10000, {});
    for (uint32_t item = 0; item < 0x10000 && in.ok; ++item) {
        if ((bits[item >> 3] & (1u << (item & 7))) == 0) {
            continue;
        }
        std::vector<CompactSrc>& vec = compact_items[item];
        while (in.ok) {
            uint8_t type = in.u8();
            if (!in.ok || type == 0xff) {
                break;
            }
            uint32_t group = compact_varint(in);
            acc = 0;
            bool rolls = type <= 0x15 && compact_roll[type] == 0;
            for (uint32_t n = 0; n < group && in.ok; ++n) {
                acc += compact_varint(in);
                uint16_t chance = 0xffff;
                if (rolls) {
                    chance = in.u16();
                    in.u16();
                }
                if (type > 0x15 || compact_prefix[type] == 0xffffffffu) {
                    continue;
                }
                uint32_t key = acc | compact_prefix[type];
                auto found = compact_keys.find(key);
                uint16_t slot = 0;
                if (found == compact_keys.end()) {
                    if (compact_make[type] == 0xff) {
                        continue;
                    }
                    slot = compact_add_obj(key, acc, compact_make[type], 0xffff, std::string());
                    if (slot == 0) {
                        continue;
                    }
                } else {
                    slot = found->second;
                }
                CompactSrc src;
                src.slot = slot;
                src.chance = chance;
                src.type = type;
                vec.push_back(src);
            }
        }
    }
    if (!in.ok) {
        compact_objs.clear();
        compact_items.clear();
        compact_keys.clear();
        return;
    }
    compact_ready = true;
    log_line("ItemLoc sources-compacted objs %zu tail %zu", compact_objs.size(), size - in.off);
}

int native_itemloc_sources_compacted(void* state) {
    if (!compact_ready || !g_is_num || !g_to_num || !g_push_num || !g_push_str || !g_push_nil) {
        return 0;
    }
    int item = arg_int(state, 1);
    if (item < 1 || item > 0xffff || static_cast<size_t>(item) >= compact_items.size()) {
        return 0;
    }
    const std::vector<CompactSrc>& sources = compact_items[static_cast<size_t>(item)];
    if (sources.empty()) {
        return 0;
    }
    int white = arg_int(state, 2);
    int black = arg_int(state, 3);
    float min_chance = compact_arg_float(state, 4, 0.f);
    float max_chance = compact_arg_float(state, 5, 100.f);
    int other = arg_int(state, 6);
    int zone = arg_int(state, 7);
    int mode = arg_int(state, 8);
    int side = (other & 4) ? 2 : 1;
    std::vector<CompactHit> hits;
    int quest_n = 0;
    int faction_or = 0;
    for (int index = 0; index < static_cast<int>(sources.size()); ++index) {
        const CompactSrc& src = sources[static_cast<size_t>(index)];
        if (!compact_keep(item, src, white, black, min_chance, max_chance, other, zone)) {
            continue;
        }
        CompactHit hit;
        hit.index = index;
        const CompactObj& obj = compact_objs[src.slot];
        if (src.type == 2 && obj.type == 2) {
            bool drop = false;
            if (other & 0x10) {
                int status = compact_quest_status(obj);
                if (status == -2) {
                    compact_chain(obj, hit.chain);
                    if (!hit.chain.empty()) {
                        if ((other & 1) && compact_chain_blocked(hit.chain)) {
                            drop = true;
                        }
                    } else if (other & 8) {
                        drop = true;
                    }
                } else if ((other & 8) && status <= 0) {
                    drop = true;
                }
            } else if ((other & 8) && compact_quest_status(obj) <= 0) {
                drop = true;
            }
            if (drop) {
                continue;
            }
            int bits = obj.flags & 3;
            faction_or |= bits == 0 ? 3 : bits;
            ++quest_n;
        }
        hits.push_back(std::move(hit));
    }
    if (faction_or == 3 && quest_n > 1) {
        std::vector<CompactHit> kept;
        kept.reserve(hits.size());
        for (CompactHit& hit : hits) {
            const CompactSrc& src = sources[static_cast<size_t>(hit.index)];
            const CompactObj& obj = compact_objs[src.slot];
            if (src.type == 2 && obj.type == 2) {
                int bits = obj.flags & 3;
                if (bits != 0 && bits != 3 && bits != side) {
                    continue;
                }
            }
            kept.push_back(std::move(hit));
        }
        hits.swap(kept);
    }
    if (hits.empty()) {
        return 0;
    }
    int src_acc = -1;
    int obj_acc = -1;
    int id_acc = -1;
    float sum = 0.f;
    int counted = 0;
    std::map<uint32_t, uint8_t> replaced;
    for (const CompactHit& hit : hits) {
        const CompactSrc& src = sources[static_cast<size_t>(hit.index)];
        int shown = src.type;
        if (shown == 0x14 || shown == 0x15) {
            shown -= 0x14;
        }
        if (src_acc == -1) {
            src_acc = shown;
        } else if (src_acc != -2 && src_acc != shown) {
            src_acc = -2;
        }
        if (src.slot == 0 || src.slot >= compact_objs.size()) {
            continue;
        }
        const CompactObj& obj = compact_objs[src.slot];
        int otype = obj.type;
        if (obj_acc == -1) {
            obj_acc = otype;
        } else if (obj_acc != -2 && obj_acc != otype) {
            obj_acc = -2;
        }
        int oid = static_cast<int>(obj.id);
        if (id_acc == -1) {
            id_acc = oid;
        } else if (id_acc != -2 && id_acc != oid) {
            if (mode > 0 && (mode >= 1000 || mode == src_acc)) {
                continue;
            }
            id_acc = -2;
        }
        for (uint32_t id : hit.chain) {
            replaced[id] = 1;
        }
        sum += compact_chance(src.chance);
        ++counted;
    }
    compact_push_num(state, src_acc);
    compact_push_num(state, obj_acc);
    compact_push_num(state, id_acc);
    g_push_str(state, compact_name(src_acc, obj_acc, id_acc));
    float average = counted > 0 ? sum / static_cast<float>(counted) : 0.f;
    g_push_num(state, static_cast<double>(average));
    if (!replaced.empty()) {
        compact_bind_lua();
    }
    if (replaced.empty() || !compact_create || !compact_rawset) {
        g_push_nil(state);
        return 6;
    }
    compact_create(state, 0, 0);
    int slot = 1;
    for (const auto& entry : replaced) {
        uint32_t id = entry.first;
        if (compact_check) {
            compact_check(state, 2);
        }
        g_push_num(state, static_cast<double>(slot));
        double wide = static_cast<double>(static_cast<int32_t>(id));
        if (static_cast<int32_t>(id) < 0) {
            wide += 4294967296.0;
        }
        g_push_num(state, wide);
        compact_rawset(state, -3);
        ++slot;
    }
    return 6;
}
