struct obj_at_row {
    uint16_t item = 0;
    uint16_t chance = 0;
    uint16_t pert = 0;
    uint8_t src = 0;
};

struct obj_at_in {
    const uint8_t* p = nullptr;
    const uint8_t* end = nullptr;
    bool ok = true;

    bool take(void* dst, size_t n) {
        if (!ok || p + n > end) {
            ok = false;
            return false;
        }
        memcpy(dst, p, n);
        p += n;
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

    void skip(size_t n) {
        if (!ok || p + n > end) {
            ok = false;
            return;
        }
        p += n;
    }

    void str() {
        while (ok && u8() != 0) {
        }
    }

    uint32_t var() {
        uint8_t byte = u8();
        uint32_t value = byte & 0x7fu;
        if ((byte & 0x80u) == 0) {
            return value;
        }
        byte = u8();
        value = ((value << 7) | (byte & 0x7fu)) & 0xffffffffu;
        if ((byte & 0x80u) == 0) {
            return value;
        }
        byte = u8();
        value = ((value << 7) | (byte & 0x7fu)) & 0xffffffffu;
        if ((byte & 0x80u) == 0) {
            return value;
        }
        byte = u8();
        value = ((value << 7) | (byte & 0x7fu)) & 0xffffffffu;
        if ((byte & 0x80u) == 0) {
            return value;
        }
        byte = u8();
        return ((value << 7) | (byte & 0x7fu)) & 0xffffffffu;
    }
};

std::map<uint32_t, std::vector<obj_at_row>> obj_at_rows;

// ʕ •ᴥ•ʔ✿ src <= 21: 0 reads chance/pert words, 1 stores 0xFFFF / 0x7FFF ✿ ʕ •ᴥ•ʔ
const uint8_t obj_at_plain[] = {
    0, 0, 1, 0, 1, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0,
};

// ʕ •ᴥ•ʔ✿ case selects the key high byte; 7 drops the row ✿ ʕ •ᴥ•ʔ
const uint8_t obj_at_case[] = {
    0, 1, 2, 3, 4, 0, 3, 5, 1, 0, 3, 3, 6, 0, 0, 7, 0, 1, 3, 8, 0, 1,
};

int obj_at_to_int(double value) {
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

double obj_at_chance(uint16_t raw) {
    if (raw == 0) {
        return 0.0;
    }
    if (raw == 0xffff) {
        return 101.0;
    }
    return static_cast<double>(static_cast<float>(static_cast<double>(raw) * (100.0 / 65000.0)));
}

double obj_at_pert(uint16_t raw) {
    if (raw & 0x8000u) {
        return static_cast<double>(static_cast<float>(static_cast<double>(raw & 0x7fffu) / 3200.0));
    }
    if (raw > 0x7ff0u) {
        return 0.0;
    }
    return static_cast<double>(static_cast<float>(static_cast<int>(raw)));
}

void obj_at_skip_extra(obj_at_in& in) {
    uint32_t flags = in.u32();
    if (flags & 1u) {
        in.u16();
    }
    if (flags & 8u) {
        in.u16();
    }
    if (flags & 0x10u) {
        in.u16();
    }
    if (flags & 0x20u) {
        in.u8();
    }
    if (flags & 0x40u) {
        in.u8();
    }
    if (flags & 0x80u) {
        in.u16();
        in.u16();
    }
    if (flags & 0x100u) {
        in.skip(4);
    }
    if (flags & 0x200u) {
        in.skip(4);
    }
    if (flags & 0x400u) {
        in.skip(4);
    }
    if (flags & 0x800u) {
        uint8_t count = in.u8();
        if (count) {
            in.skip(static_cast<size_t>(count) * 2u);
        }
    }
    if (flags & 0x1000u) {
        in.u16();
    }
    if (flags & 0x2000u) {
        uint8_t count = in.u8();
        if (count) {
            in.skip(static_cast<size_t>(count) * 2u);
        }
    }
    if (flags & 0x100000u) {
        uint8_t count = in.u8();
        if (count) {
            in.skip(static_cast<size_t>(count) * 2u);
        }
    }
    if (flags & 0x200000u) {
        uint8_t count = in.u8();
        if (count) {
            in.skip(static_cast<size_t>(count) * 2u);
        }
    }
    if (flags & 0x400000u) {
        uint8_t count = in.u8();
        if (count) {
            in.skip(static_cast<size_t>(count) * 2u);
        }
    }
    if (flags & 0x800000u) {
        uint8_t count = in.u8();
        if (count) {
            in.skip(static_cast<size_t>(count) * 2u);
        }
    }
}

bool obj_at_bit(const uint8_t* bits, int id) {
    return (bits[id >> 3] >> (id & 7)) & 1;
}

void load_itemloc_obj_at(const uint8_t* data, size_t size) {
    obj_at_rows.clear();
    obj_at_in in{data, data + size, true};
    uint32_t version = in.u32();
    if (!in.ok || version < 1 || version > 5) {
        log_line("WoWExtItemLoc.bin obj-at version %u rejected", version);
        return;
    }
    in.u32();
    if (!in.ok || in.p + 0x2000 > in.end) {
        return;
    }
    const uint8_t* bits = in.p;
    in.skip(0x2000);

    std::vector<char> has0(65536, 0);
    std::vector<std::vector<obj_at_row>> low(65536);
    std::map<uint32_t, uint8_t> has_hi;
    std::map<uint32_t, std::vector<obj_at_row>> high;

    for (int id = 0; id < 65536 && in.ok; ++id) {
        if (!obj_at_bit(bits, id)) {
            continue;
        }
        in.str();
        uint16_t word = in.u16();
        if (word != 0xffff) {
            in.u8();
        }
        if (version >= 2) {
            uint8_t mask = in.u8();
            if (mask) {
                for (int bit = 0; bit < 3; ++bit) {
                    if (mask & (1u << bit)) {
                        in.u16();
                    }
                }
            }
        }
        has0[static_cast<size_t>(id)] = 1;
    }

    uint32_t count = in.u32();
    uint32_t acc = 0;
    for (uint32_t i = 0; i < count && in.ok; ++i) {
        acc += in.var();
        in.str();
        uint16_t word = in.u16();
        if (word != 0xffff) {
            in.u8();
        }
        has_hi[acc | 0x01000000u] = 1;
    }

    count = in.u32();
    if (version >= 5) {
        in.u32();
    }
    acc = 0;
    for (uint32_t i = 0; i < count && in.ok; ++i) {
        acc += in.var();
        in.str();
        in.u16();
        if (version >= 3) {
            uint8_t mask = in.u8();
            if (mask & 4u) {
                obj_at_skip_extra(in);
            }
        }
        has_hi[acc | 0x02000000u] = 1;
    }

    if (!in.ok || in.p + 0x2000 > in.end) {
        log_line("WoWExtItemLoc.bin obj-at truncated at %zu", static_cast<size_t>(in.p - data));
        return;
    }
    bits = in.p;
    in.skip(0x2000);

    size_t drops = 0;
    for (int item = 0; item < 65536 && in.ok; ++item) {
        if (!obj_at_bit(bits, item)) {
            continue;
        }
        while (in.ok) {
            uint8_t src = in.u8();
            if (!in.ok || src == 0xff) {
                break;
            }
            uint32_t n = in.var();
            bool plain = src > 21 || obj_at_plain[src] != 0;
            int which = obj_at_case[src > 21 ? 0 : src];
            if (src > 21) {
                which = 7;
            }
            uint32_t mask = 0;
            bool make = false;
            bool known = false;
            switch (which) {
            case 0:
                known = true;
                break;
            case 1:
                mask = 0x01000000u;
                known = true;
                break;
            case 2:
                mask = 0x02000000u;
                known = true;
                break;
            case 3:
                mask = 0x03000000u;
                make = true;
                break;
            case 4:
                mask = 0x04000000u;
                known = true;
                break;
            case 5:
                mask = 0x05000000u;
                make = true;
                break;
            case 6:
                mask = 0x06000000u;
                make = true;
                break;
            case 8:
                mask = 0x15000000u;
                make = true;
                break;
            default:
                break;
            }
            acc = 0;
            for (uint32_t i = 0; i < n && in.ok; ++i) {
                acc += in.var();
                obj_at_row row;
                row.src = src;
                row.item = static_cast<uint16_t>(item);
                if (plain) {
                    row.chance = 0xffff;
                    row.pert = 0x7fff;
                } else {
                    row.chance = in.u16();
                    row.pert = in.u16();
                }
                if (!in.ok || which == 7) {
                    continue;
                }
                uint32_t key = acc | mask;
                if (known) {
                    if (which == 0) {
                        if (key >= 65536 || !has0[key]) {
                            continue;
                        }
                        low[key].push_back(row);
                    } else if (!has_hi.count(key)) {
                        continue;
                    } else {
                        high[key].push_back(row);
                    }
                } else if (make) {
                    high[key].push_back(row);
                }
                ++drops;
            }
        }
    }

    if (!in.ok) {
        log_line("WoWExtItemLoc.bin obj-at truncated at %zu", static_cast<size_t>(in.p - data));
        return;
    }

    std::map<uint32_t, std::vector<obj_at_row>> built;
    for (uint32_t id = 0; id < 65536; ++id) {
        if (!low[id].empty()) {
            built.emplace(id, std::move(low[id]));
        }
    }
    for (auto& entry : high) {
        if (!entry.second.empty()) {
            built.emplace(entry.first, std::move(entry.second));
        }
    }
    obj_at_rows.swap(built);
    log_line("WoWExtItemLoc.bin obj-at %zu bytes version %u keys %zu drops %zu",
             size, version, obj_at_rows.size(), drops);
}

// ʕ •ᴥ•ʔ✿ key is (arg1<<24)|arg2; index is 1-based into the load-order list ✿ ʕ •ᴥ•ʔ
int native_itemloc_obj_at(void* state) {
    if (!g_is_num || !g_to_num || !g_is_num(state, 1) || !g_is_num(state, 2) || !g_is_num(state, 3)) {
        return 0;
    }
    int cat = obj_at_to_int(g_to_num(state, 1));
    int id = obj_at_to_int(g_to_num(state, 2));
    int index = obj_at_to_int(g_to_num(state, 3));
    if (index <= 0) {
        return 0;
    }
    uint32_t key = (static_cast<uint32_t>(cat) << 24) | static_cast<uint32_t>(id);
    auto found = obj_at_rows.find(key);
    if (found == obj_at_rows.end()) {
        return 0;
    }
    size_t slot = static_cast<size_t>(index - 1);
    if (slot >= found->second.size()) {
        return 0;
    }
    const obj_at_row& row = found->second[slot];
    g_push_num(state, row.src);
    g_push_num(state, row.item);
    g_push_num(state, obj_at_chance(row.chance));
    g_push_num(state, obj_at_pert(row.pert));
    return 4;
}
