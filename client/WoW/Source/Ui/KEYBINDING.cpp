#include "KEYBINDING.h"
#include <cstring>

// VERIFY: 4c0baa0c manual
// OFFSET: 0x0055E6A0
KEYBINDING::KEYBINDING() : m_flags(0) {
    memset(this->m_data, 0, sizeof(this->m_data));
}

// VERIFY: 890ceb18 auto
// OFFSET: 0x0055E130
KEYBINDING& KEYBINDING::operator=(const KEYBINDING& source) {
    this->m_flags = source.m_flags;

    for (int32_t i = 0; i < 4; i++) {
        this->m_data[i].m_index = source.m_data[i].m_index;
        this->m_data[i].m_command.Copy(source.m_data[i].m_command);
    }

    return *this;
}
