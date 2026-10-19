constexpr int kRecipesLimit = 0x1fbd0;
constexpr uint32_t kRecipesRowSize = 0x2b8;
constexpr int kRecipesAttr = 0x10;
constexpr int kRecipesReagent = 0xd0;
constexpr int kRecipesReagentCount = 0xf0;
constexpr int kRecipesEffect = 0x11c;
constexpr int kRecipesItemType = 0x1ac;
constexpr int kRecipesName = 0x220;
constexpr uint32_t kRecipesTradeBit = 0x20;
constexpr int kRecipesCharRace = 0x178;
constexpr int kRecipesCharClass = 0x179;

using RecipesDecompressFn = uint8_t* (*)(const uint8_t*, uint32_t, uint8_t*);
using RecipesActiveFn = uint64_t (*)();
using RecipesObjectFn = void* (*)(uint64_t, uint32_t, const char*, int);
using RecipesAbilityFn = void* (*)(void*, int);
using RecipesBagFn = void* (*)(void*);
using RecipesCountFn = int32_t (*)(void*, int32_t, uint32_t);
using RecipesTableFn = void (*)(void*, int, int);
using RecipesRawFn = void (*)(void*, int);
using RecipesCheckFn = int (*)(void*, int);

struct RecipesBind {
    const uint8_t* spells = nullptr;
    const int32_t* compress = nullptr;
    RecipesDecompressFn decompress = nullptr;
    const uint8_t* known = nullptr;
    const uint8_t* character = nullptr;
    RecipesActiveFn active = nullptr;
    RecipesObjectFn object = nullptr;
    RecipesAbilityFn ability = nullptr;
    RecipesBagFn bag = nullptr;
    RecipesCountFn count = nullptr;
    RecipesTableFn table = nullptr;
    RecipesRawFn rawset = nullptr;
    RecipesCheckFn check = nullptr;
};

struct RecipesHit {
    int32_t id = 0;
    int32_t skill = 0;
    std::string spell;
    std::string made;
};

RecipesBind& recipes_bind() {
    static RecipesBind bind;
    static bool ready = false;
    if (ready) {
        return bind;
    }
    ready = true;
    void* found = nullptr;
    if (syn_resolve("g_spellDB", &found)) {
        bind.spells = static_cast<const uint8_t*>(found);
    }
    found = nullptr;
    if (syn_resolve("g_clientDbPerformCompress", &found)) {
        bind.compress = static_cast<const int32_t*>(found);
    }
    found = nullptr;
    if (syn_resolve("_Z13DecompressRowPKhjPh", &found)) {
        bind.decompress = reinterpret_cast<RecipesDecompressFn>(found);
    }
    found = nullptr;
    if (syn_resolve("s_knownSpellBits", &found)) {
        bind.known = static_cast<const uint8_t*>(found);
    }
    found = nullptr;
    if (syn_resolve("_ZN14ClientServices15s_characterInfoE", &found)) {
        bind.character = static_cast<const uint8_t*>(found);
    }
    found = nullptr;
    if (syn_resolve("_Z25ClntObjMgrGetActivePlayerv", &found)) {
        bind.active = reinterpret_cast<RecipesActiveFn>(found);
    }
    found = nullptr;
    if (syn_resolve("_Z19ClntObjMgrObjectPtr7WOWGUID8TypeMaskPKci", &found)) {
        bind.object = reinterpret_cast<RecipesObjectFn>(found);
    }
    found = nullptr;
    if (syn_resolve("_ZN8CGUnit_C13LookupAbilityEi", &found)) {
        bind.ability = reinterpret_cast<RecipesAbilityFn>(found);
    }
    found = nullptr;
    if (syn_resolve("_ZNK10CGPlayer_C6GetBagEv", &found)) {
        bind.bag = reinterpret_cast<RecipesBagFn>(found);
    }
    found = nullptr;
    if (syn_resolve("_ZN7CGBag_C16GetItemTypeCountEij", &found)) {
        bind.count = reinterpret_cast<RecipesCountFn>(found);
    }
    found = nullptr;
    if (syn_resolve("lua_createtable", &found)) {
        bind.table = reinterpret_cast<RecipesTableFn>(found);
    }
    found = nullptr;
    if (syn_resolve("lua_rawset", &found)) {
        bind.rawset = reinterpret_cast<RecipesRawFn>(found);
    }
    found = nullptr;
    if (syn_resolve("lua_checkstack", &found)) {
        bind.check = reinterpret_cast<RecipesCheckFn>(found);
    }
    return bind;
}

int32_t recipes_i32(const uint8_t* row, int offset) {
    int32_t value = 0;
    memcpy(&value, row + offset, 4);
    return value;
}

const char* recipes_ptr(const uint8_t* row, int offset) {
    const char* text = nullptr;
    memcpy(&text, row + offset, sizeof(text));
    return text;
}

int recipes_lower(unsigned char c) {
    if (c >= 'A' && c <= 'Z') {
        return c + 32;
    }
    return c;
}

int recipes_icmp(const char* left, const char* right) {
    const unsigned char* a = reinterpret_cast<const unsigned char*>(left ? left : "");
    const unsigned char* b = reinterpret_cast<const unsigned char*>(right ? right : "");
    while (*a && recipes_lower(*a) == recipes_lower(*b)) {
        ++a;
        ++b;
    }
    return recipes_lower(*a) - recipes_lower(*b);
}

const char* recipes_ifind(const char* hay, const char* needle) {
    if (!hay || !needle || !needle[0]) {
        return nullptr;
    }
    for (const char* p = hay; *p; ++p) {
        const unsigned char* a = reinterpret_cast<const unsigned char*>(p);
        const unsigned char* b = reinterpret_cast<const unsigned char*>(needle);
        while (*a && *b && recipes_lower(*a) == recipes_lower(*b)) {
            ++a;
            ++b;
        }
        if (!*b) {
            return p;
        }
    }
    return nullptr;
}

int recipes_round_int(double value) {
    if (!(value > -2147483648.0)) {
        return static_cast<int>(0x80000000);
    }
    if (!(value < 2147483647.0)) {
        return 0x7fffffff;
    }
    double rounded = value >= 0.0
        ? static_cast<double>(static_cast<long long>(value + 0.5))
        : static_cast<double>(static_cast<long long>(value - 0.5));
    if (!(rounded > -2147483648.0)) {
        return static_cast<int>(0x80000000);
    }
    if (!(rounded < 2147483647.0)) {
        return 0x7fffffff;
    }
    return static_cast<int>(rounded);
}

bool recipes_opt_int(void* state, int index, int* out) {
    if (!g_is_num(state, index)) {
        return false;
    }
    *out = recipes_round_int(g_to_num(state, index));
    return true;
}

const uint8_t* recipes_spell(int id, uint8_t* storage) {
    const RecipesBind& bind = recipes_bind();
    if (!bind.spells || id < 0 || id >= kRecipesLimit) {
        return nullptr;
    }
    int32_t max_id = recipes_i32(bind.spells, 0x10);
    int32_t min_id = recipes_i32(bind.spells, 0x14);
    if (id < min_id || id > max_id) {
        return nullptr;
    }
    const uint8_t* const* by_id = nullptr;
    memcpy(&by_id, bind.spells + 0x30, sizeof(by_id));
    if (!by_id) {
        return nullptr;
    }
    const uint8_t* raw = by_id[id - min_id];
    if (!raw) {
        return nullptr;
    }
    if (bind.compress && *bind.compress) {
        if (!bind.decompress) {
            return nullptr;
        }
        bind.decompress(raw, kRecipesRowSize, storage);
        return storage;
    }
    return raw;
}

void* recipes_player() {
    const RecipesBind& bind = recipes_bind();
    if (!bind.active || !bind.object) {
        return nullptr;
    }
    return bind.object(bind.active(), 0x10, "test:1", 0xa0);
}

const ItemRow* recipes_item(int id) {
    if (id <= 0 || static_cast<uint32_t>(id) > 0xffffu) {
        return nullptr;
    }
    return item_row(id);
}

bool recipes_creates(int32_t effect) {
    return effect == 0x18 || effect == 0x9d || effect == 0x3b;
}

int32_t recipes_stock(int item, uint32_t flags, bool extra) {
    const RecipesBind& bind = recipes_bind();
    void* player = recipes_player();
    void* bag = player && bind.bag ? bind.bag(player) : nullptr;
    int32_t total = bag && bind.count ? bind.count(bag, item, flags) : 0;
    if (!extra) {
        return total;
    }
    int rounded = recipes_round_int(game_num(13, item));
    if (rounded > 0) {
        total += rounded;
    }
    return total;
}

int32_t recipes_craftable(const uint8_t* spell) {
    int32_t crafts[8] = {};
    for (int i = 0; i < 8; ++i) {
        int32_t item = recipes_i32(spell, kRecipesReagent + i * 4);
        uint32_t need = static_cast<uint32_t>(recipes_i32(spell, kRecipesReagentCount + i * 4));
        if (!item || need == 0) {
            continue;
        }
        uint32_t made = static_cast<uint32_t>(recipes_stock(item, 0, true)) / need;
        if (static_cast<int32_t>(made) <= 0) {
            return 0;
        }
        crafts[i] = static_cast<int32_t>(made);
    }
    int32_t best = 0x7fffffff;
    for (int i = 0; i < 8; ++i) {
        if (crafts[i] > 0 && crafts[i] < best) {
            best = crafts[i];
        }
    }
    return best == 0x7fffffff ? 0 : best;
}

uint8_t recipes_char_byte(int offset) {
    const uint8_t* info = recipes_bind().character;
    if (!info) {
        return 0;
    }
    return info[offset];
}

uint32_t recipes_class_bits() {
    uint32_t bits = 0;
    int limit = g_class_count < 10 ? g_class_count : 10;
    for (int i = 0; i < limit; ++i) {
        int id = g_classes[i].id;
        if (id >= 1 && id <= 11) {
            bits |= 1u << (id - 1);
        }
    }
    if (bits) {
        return bits;
    }
    int klass = recipes_char_byte(kRecipesCharClass);
    if (klass >= 1 && klass <= 11) {
        return 1u << (klass - 1);
    }
    return 0;
}

bool recipes_armor_ok(int sub, int level, int player) {
    int primary = 1;
    int alt = 0;
    if (player == 1 || player == 2 || player == 6) {
        primary = 4;
        if (static_cast<uint16_t>(level) < 0x2d) {
            alt = 3;
        }
    } else if (player == 4 || player == 11) {
        primary = 2;
    } else if (player == 3 || player == 7) {
        primary = static_cast<uint16_t>(level) < 0x2d ? 2 : 3;
    }
    return sub == primary || (alt && sub == alt);
}

bool recipes_class_mask(int player, uint32_t mask, int limit) {
    if (player < 1 || player > limit) {
        return false;
    }
    return (mask & (1u << (player - 1))) != 0;
}

bool recipes_relic_ok(int sub, int player) {
    if (sub == 7) {
        return player == 2;
    }
    if (sub == 8) {
        return player == 11;
    }
    if (sub == 9) {
        return player == 7;
    }
    if (sub == 10) {
        return player == 6;
    }
    return true;
}

bool recipes_weapon_ok(int klass, int inv, int sub, int player) {
    if (inv == 22) {
        if (!recipes_class_mask(player, 0x6d, 7)) {
            return false;
        }
    } else if (inv == 14) {
        if (player != 1 && player != 2 && player != 7) {
            return false;
        }
    }
    bool fit = true;
    if (klass == 2) {
        if (sub == 19) {
            fit = player == 5 || player == 8 || player == 9;
        } else if (sub == 2 || sub == 3 || sub == 16 || sub == 18) {
            fit = player == 1 || player == 3 || player == 4;
        } else if (sub == 6) {
            fit = recipes_class_mask(player, 0x427, 11);
        } else if (sub == 5) {
            fit = recipes_class_mask(player, 0x463, 11);
        } else if (sub == 4) {
            fit = recipes_class_mask(player, 0x47b, 11);
        } else if (sub == 15) {
            fit = recipes_class_mask(player, 0x5dd, 11);
        } else if (sub == 10) {
            fit = recipes_class_mask(player, 0x5d5, 11);
        } else if (sub == 7) {
            fit = recipes_class_mask(player, 0xaf, 9);
        } else if (sub == 1) {
            fit = recipes_class_mask(player, 0x67, 7);
        } else if (sub == 8) {
            fit = recipes_class_mask(player, 0x27, 6);
        } else if (sub == 0) {
            fit = recipes_class_mask(player, 0x6f, 7);
        } else if (sub == 13) {
            fit = recipes_class_mask(player, 0x44d, 11);
        }
    }
    if (!fit) {
        return false;
    }
    if (inv != 28) {
        return true;
    }
    return recipes_relic_ok(sub, player);
}

// ʕ •ᴥ•ʔ✿ fixed call used by flags 1 and 4: report mode, current classes, current race ✿ ʕ •ᴥ•ʔ
int recipes_use(const ItemRow* row) {
    if (!row || (row->tag_word & 0x40) == 0) {
        return -1;
    }
    int inv = static_cast<int8_t>(row->inv);
    static const uint8_t kRejectInv[28] = {
        1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1, 0, 0, 1,
    };
    if (inv >= 0 && inv <= 27 && kRejectInv[inv]) {
        return -1;
    }
    if ((row->flags & 0x01) || (row->flags & 0x08)) {
        return -1;
    }
    uint32_t bits = recipes_class_bits();
    int klass = static_cast<int8_t>(row->klass);
    int sub = static_cast<int8_t>(row->sub);
    if (inv != 16 && bits) {
        if (klass == 4 && sub >= 1 && sub <= 4) {
            bool armor = false;
            for (int player = 1; player <= 11; ++player) {
                if ((bits & (1u << (player - 1))) && recipes_armor_ok(sub, static_cast<int>(row->level), player)) {
                    armor = true;
                    break;
                }
            }
            if (!armor) {
                return 0;
            }
        }
        bool fit = false;
        for (int player = 1; player <= 11; ++player) {
            if ((bits & (1u << (player - 1))) && recipes_weapon_ok(klass, inv, sub, player)) {
                fit = true;
                break;
            }
        }
        if (!fit) {
            return 0;
        }
    }
    uint32_t class_mask = row->class_mask & 0x5ffu;
    if (class_mask && class_mask != 0x5ffu && (bits & class_mask) == 0) {
        return 0;
    }
    uint32_t race_mask = row->race_mask & 0x6ffu;
    if (race_mask && race_mask != 0x6ffu) {
        int race = recipes_char_byte(kRecipesCharRace);
        uint32_t bit = race >= 1 && race <= 31 ? 1u << (race - 1) : 0;
        if ((race_mask & bit) == 0) {
            return 0;
        }
    }
    return 1;
}

bool recipes_tick(double value) {
    double shifted = value + 0.01;
    if (!(shifted > -2147483648.0) || !(shifted < 2147483647.0)) {
        return true;
    }
    return static_cast<int>(shifted) != 0;
}

bool recipes_attuned(const ItemRow* row, int item, int slot) {
    if (!row || item <= 0 || (row->tag_word & 0x40) == 0) {
        return false;
    }
    for (int s = 0; s <= 3; ++s) {
        if (slot >= 0 && slot != s) {
            continue;
        }
        int key = (s << 24) | item;
        if (game_num(11, key) >= 100.0) {
            return true;
        }
        if ((row->tag_word & 0x1000u) == 0) {
            continue;
        }
        if (recipes_tick(game_num(16, key)) || recipes_tick(game_num(17, key)) || recipes_tick(game_num(20, key))) {
            return true;
        }
    }
    return false;
}

bool recipes_name_hit(const RecipesHit& hit, const char* filter) {
    if (recipes_ifind(hit.spell.c_str(), filter)) {
        return true;
    }
    return !hit.made.empty() && recipes_ifind(hit.made.c_str(), filter);
}

bool recipes_reagent_hit(const uint8_t* spell, const char* filter) {
    for (int i = 0; i < 8; ++i) {
        int32_t item = recipes_i32(spell, kRecipesReagent + i * 4);
        int32_t count = recipes_i32(spell, kRecipesReagentCount + i * 4);
        if (!item || static_cast<uint32_t>(item) > 0xffffu || count == 0) {
            continue;
        }
        const ItemRow* row = recipes_item(item);
        if (row && recipes_ifind(row->name.c_str(), filter)) {
            return true;
        }
    }
    return false;
}

bool recipes_sort_before(const RecipesHit& left, const RecipesHit& right, int mode) {
    if (mode == 1) {
        return recipes_icmp(left.spell.c_str(), right.spell.c_str()) < 0;
    }
    if (mode == -1) {
        return recipes_icmp(left.spell.c_str(), right.spell.c_str()) > 0;
    }
    if (mode == 2) {
        return recipes_icmp(left.made.c_str(), right.made.c_str()) < 0;
    }
    if (mode == -2) {
        return recipes_icmp(left.made.c_str(), right.made.c_str()) > 0;
    }
    if (mode == 3 || mode == -3) {
        if (left.skill != right.skill) {
            return mode == 3 ? left.skill < right.skill : left.skill > right.skill;
        }
        return recipes_icmp(left.spell.c_str(), right.spell.c_str()) < 0;
    }
    return false;
}

const uint32_t* recipes_known_words() {
    const uint8_t* known = recipes_bind().known;
    if (!known) {
        return nullptr;
    }
    const uint32_t* words = nullptr;
    memcpy(&words, known + 0x10, sizeof(words));
    return words;
}

// ʕ •ᴥ•ʔ✿ skill, keep flags, drop flags, sort, name, attune slot, item class, subclass, inv; one table of spell ids ✿ ʕ •ᴥ•ʔ
int native_get_profession_recipes(void* state) {
    const RecipesBind& bind = recipes_bind();
    if (!bind.table || !bind.rawset || !g_push_num || !g_is_num || !g_to_num) {
        return 0;
    }
    bind.table(state, 0, 0);
    int skill = -1;
    int keep = 0;
    int drop = 0;
    int sort_mode = 0;
    int attune_slot = 0;
    int want_class = -1;
    int want_sub = -1;
    int want_inv = -1;
    recipes_opt_int(state, 1, &skill);
    recipes_opt_int(state, 2, &keep);
    recipes_opt_int(state, 3, &drop);
    recipes_opt_int(state, 4, &sort_mode);
    recipes_opt_int(state, 6, &attune_slot);
    recipes_opt_int(state, 7, &want_class);
    recipes_opt_int(state, 8, &want_sub);
    recipes_opt_int(state, 9, &want_inv);
    const char* filter = nullptr;
    if (g_is_str && g_to_str && g_is_str(state, 5)) {
        filter = g_to_str(state, 5, nullptr);
        if (filter && !filter[0]) {
            filter = nullptr;
        }
    }
    int flags = keep | drop;
    std::vector<RecipesHit> hits;
    const uint32_t* words = recipes_known_words();
    int32_t max_id = bind.spells ? recipes_i32(bind.spells, 0x10) : -1;
    int32_t word_count = (max_id + 31) >> 5;
    alignas(8) uint8_t storage[kRecipesRowSize];
    if (words && word_count > 0) {
        for (int32_t word = 0; word < word_count; ++word) {
            uint32_t bits = words[word];
            if (!bits) {
                continue;
            }
            for (int half = 0; half < 2; ++half) {
                if ((bits & 0xffffu) == 0) {
                    bits >>= 16;
                    continue;
                }
                for (int bit = 0; bit < 16; ++bit) {
                    if ((bits & (1u << bit)) == 0) {
                        continue;
                    }
                    uint32_t spell_id = static_cast<uint32_t>(word) * 32u + static_cast<uint32_t>(half) * 16u + static_cast<uint32_t>(bit);
                    if (spell_id > 0x1fbcfu) {
                        continue;
                    }
                    const uint8_t* spell = recipes_spell(static_cast<int>(spell_id), storage);
                    if (!spell || (static_cast<uint8_t>(recipes_i32(spell, kRecipesAttr)) & kRecipesTradeBit) == 0) {
                        continue;
                    }
                    void* player = recipes_player();
                    const uint8_t* ability = player && bind.ability
                        ? static_cast<const uint8_t*>(bind.ability(player, recipes_i32(spell, 0)))
                        : nullptr;
                    if (!ability) {
                        continue;
                    }
                    if (skill >= 0 && recipes_i32(ability, 4) != skill) {
                        continue;
                    }
                    int32_t created = 0;
                    if (recipes_creates(recipes_i32(spell, kRecipesEffect))) {
                        created = recipes_i32(spell, kRecipesItemType);
                    }
                    const ItemRow* item = created ? recipes_item(created) : nullptr;
                    if (want_class >= 0 && (!item || static_cast<int8_t>(item->klass) != want_class)) {
                        continue;
                    }
                    if (want_sub >= 0 && (!item || static_cast<int8_t>(item->sub) != want_sub)) {
                        continue;
                    }
                    if (want_inv >= 0) {
                        if (!item) {
                            continue;
                        }
                        uint8_t inv = item->inv;
                        // ʕ •ᴥ•ʔ✿ inventory 5 also accepts robe (20) ✿ ʕ •ᴥ•ʔ
                        bool inv_ok = want_inv == 5 ? (inv == 5 || inv == 0x14) : static_cast<int8_t>(inv) == want_inv;
                        if (!inv_ok) {
                            continue;
                        }
                    }
                    if ((flags & 0x1) || (flags & 0x4)) {
                        if (!item) {
                            continue;
                        }
                        int status = recipes_use(item);
                        if ((keep & 0x1) && status <= 0) {
                            continue;
                        }
                        if ((drop & 0x1) && status > 0) {
                            continue;
                        }
                        if ((keep & 0x4) && status < 0) {
                            continue;
                        }
                        if ((drop & 0x4) && status >= 0) {
                            continue;
                        }
                    }
                    if (flags & 0x40) {
                        if (!item) {
                            continue;
                        }
                        bool attuned = recipes_attuned(item, created, attune_slot);
                        if ((keep & 0x40) && !attuned) {
                            continue;
                        }
                        if ((drop & 0x40) && attuned) {
                            continue;
                        }
                    }
                    RecipesHit hit;
                    hit.id = recipes_i32(spell, 0);
                    hit.skill = recipes_i32(ability, 0x28);
                    const char* spell_name = recipes_ptr(spell, kRecipesName);
                    hit.spell = spell_name ? spell_name : "";
                    if (item) {
                        hit.made = item->name;
                    }
                    if (filter && !recipes_name_hit(hit, filter) && ((keep & 0x8) == 0 || !recipes_reagent_hit(spell, filter))) {
                        continue;
                    }
                    if (flags & 0x10) {
                        // ʕ •ᴥ•ʔ✿ bit 0x80 on the combined flags skips the owned-item test when nothing was created ✿ ʕ •ᴥ•ʔ
                        if (!item) {
                            if ((flags & 0x80) == 0) {
                                continue;
                            }
                        } else {
                            int32_t owned = recipes_stock(created, 8, false);
                            if ((keep & 0x10) && owned <= 0) {
                                continue;
                            }
                            if ((drop & 0x10) && owned > 0) {
                                continue;
                            }
                        }
                    }
                    if (flags & 0x20) {
                        int32_t made = recipes_craftable(spell);
                        if ((keep & 0x20) && made <= 0) {
                            continue;
                        }
                        if ((drop & 0x20) && made > 0) {
                            continue;
                        }
                    }
                    hits.push_back(std::move(hit));
                }
                bits >>= 16;
            }
        }
    }
    if (sort_mode == 1 || sort_mode == -1 || sort_mode == 2 || sort_mode == -2 || sort_mode == 3 || sort_mode == -3) {
        std::sort(hits.begin(), hits.end(), [&](const RecipesHit& left, const RecipesHit& right) {
            return recipes_sort_before(left, right, sort_mode);
        });
    }
    int index = 1;
    for (const RecipesHit& hit : hits) {
        if (bind.check) {
            bind.check(state, 2);
        }
        g_push_num(state, static_cast<double>(index));
        g_push_num(state, static_cast<double>(static_cast<uint32_t>(hit.id)));
        bind.rawset(state, -3);
        ++index;
    }
    return 1;
}
