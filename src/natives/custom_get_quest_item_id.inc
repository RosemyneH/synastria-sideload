struct QuestItemInfo {
    int32_t item_id;
    int32_t display_id;
    int32_t count;
};

struct QuestItemSlot {
    QuestItemInfo reward;
    QuestItemInfo choice;
    QuestItemInfo required;
};

QuestItemSlot* quest_item_slots = nullptr;
bool quest_item_bound = false;

void quest_item_bind() {
    if (quest_item_bound) {
        return;
    }
    quest_item_bound = true;
    void* found = nullptr;
    if (syn_resolve("_ZN11CGQuestInfo12m_questItemsE", &found)) {
        quest_item_slots = static_cast<QuestItemSlot*>(found);
    }
}

bool quest_item_same(const char* text, const char* kind) {
    for (;;) {
        unsigned char left = static_cast<unsigned char>(*text++);
        unsigned char right = static_cast<unsigned char>(*kind++);
        if (left >= 'A' && left <= 'Z') {
            left = static_cast<unsigned char>(left + 0x20);
        }
        if (right >= 'A' && right <= 'Z') {
            right = static_cast<unsigned char>(right + 0x20);
        }
        if (left != right) {
            return false;
        }
        if (left == 0) {
            return true;
        }
    }
}

int32_t quest_item_at(const char* type, uint32_t index) {
    if (!quest_item_slots || index >= 6) {
        return 0;
    }
    const QuestItemSlot& slot = quest_item_slots[index];
    if (quest_item_same(type, "reward")) {
        return slot.reward.item_id;
    }
    if (quest_item_same(type, "choice")) {
        return slot.choice.item_id;
    }
    if (quest_item_same(type, "required")) {
        return slot.required.item_id;
    }
    return 0;
}

bool quest_item_index(double value, int* out) {
    if (!(value > -2147483648.0)) {
        return false;
    }
    if (!(value < 2147483647.0)) {
        *out = 0x7fffffff;
        return true;
    }
    double shifted = value >= 0.0 ? value + 0.5 : value - 0.5;
    if (!(shifted > -2147483648.0)) {
        return false;
    }
    if (!(shifted < 2147483647.0)) {
        *out = 0x7fffffff;
        return true;
    }
    *out = static_cast<int>(shifted);
    return true;
}

// ʕ •ᴥ•ʔ✿ type string and 1-based index. Round half away from 0. Push the item id from CGQuestInfo::m_questItems reward, choice, or required. ✿ ʕ •ᴥ•ʔ
int native_get_quest_item_id(void* state) {
    if (!g_is_str || !g_to_str || !g_is_num || !g_to_num || !g_push_num) {
        return 0;
    }
    if (!g_is_str(state, 1) || !g_is_num(state, 2)) {
        return 0;
    }
    const char* type = g_to_str(state, 1, nullptr);
    int index = 0;
    if (!quest_item_index(g_to_num(state, 2), &index)) {
        return 0;
    }
    if (index < 1) {
        return 0;
    }
    if (!type || type[0] == 0) {
        return 0;
    }
    quest_item_bind();
    g_push_num(state, static_cast<double>(quest_item_at(type, static_cast<uint32_t>(index - 1))));
    return 1;
}
