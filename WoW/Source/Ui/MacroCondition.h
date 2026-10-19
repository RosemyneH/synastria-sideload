#ifndef WOW_SOURCE_UI_MACROCONDITION_H
#define WOW_SOURCE_UI_MACROCONDITION_H

#include <cstddef>
#include <cstdint>
#include "Engine/Source/Storm/H/stpl.h"
#include "WoW/Source/Ui/MacroSubstring.h"

class MacroCondition {
    public:
    /* 0000 */ int32_t m_type;
    /* 0004 */ TSGrowableArray<MacroSubstring> m_args;
    /* 0014 */ int32_t m_negate;

    MacroCondition();
    void Parse(const char* start, const char* end);
};

// INLINED
inline MacroCondition::MacroCondition() {
    this->m_type = 0;
    this->m_negate = 0;
}

#endif
