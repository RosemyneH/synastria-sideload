struct SkillIndexBind {
    const int32_t* num_lines = nullptr;
    const uint8_t* lines = nullptr;
};

SkillIndexBind& skill_index_bind() {
    static SkillIndexBind bind;
    static bool ready = false;
    if (!ready) {
        ready = true;
        void* found = nullptr;
        if (syn_resolve("_ZN11CGSkillInfo10m_numLinesE", &found)) {
            bind.num_lines = static_cast<const int32_t*>(found);
        }
        found = nullptr;
        if (syn_resolve("_ZN11CGSkillInfo7m_linesE", &found)) {
            bind.lines = static_cast<const uint8_t*>(found);
        }
    }
    return bind;
}

bool skill_index_id(double value, int* out) {
    constexpr double k_min = -2147483648.0;
    constexpr double k_max = 2147483647.0;
    if (!(value > k_min)) {
        return false;
    }
    if (!(value < k_max)) {
        *out = 0x7fffffff;
        return true;
    }
    double rounded = value >= 0.0
        ? static_cast<double>(static_cast<long long>(value + 0.5))
        : static_cast<double>(static_cast<long long>(value - 0.5));
    if (!(rounded > k_min)) {
        return false;
    }
    if (!(rounded < k_max)) {
        *out = 0x7fffffff;
        return true;
    }
    int id = static_cast<int>(rounded);
    if (id <= 0) {
        return false;
    }
    *out = id;
    return true;
}

// ʕ •ᴥ•ʔ✿ one skill line id; 1-based index in CGSkillInfo::m_lines, or nothing ✿ ʕ •ᴥ•ʔ
int native_get_skill_index(void* state) {
    if (!g_is_num || !g_to_num || !g_push_num || !g_is_num(state, 1)) {
        return 0;
    }
    int id = 0;
    if (!skill_index_id(g_to_num(state, 1), &id)) {
        return 0;
    }
    const SkillIndexBind& bind = skill_index_bind();
    if (!bind.num_lines || !bind.lines) {
        return 0;
    }
    int count = *bind.num_lines;
    if (count <= 0) {
        return 0;
    }
    const void* data = nullptr;
    memcpy(&data, bind.lines + 8, sizeof(data));
    if (!data) {
        return 0;
    }
    for (int index = 0; index < count; ++index) {
        const void* line = nullptr;
        memcpy(&line, static_cast<const char*>(data) + static_cast<size_t>(index) * sizeof(void*), sizeof(line));
        if (!line) {
            continue;
        }
        int32_t skill = 0;
        memcpy(&skill, line, sizeof(skill));
        if (skill != id) {
            continue;
        }
        g_push_num(state, static_cast<double>(index + 1));
        return 1;
    }
    return 0;
}
