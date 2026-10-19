#include <cstdint>
#include <cstring>
#include "Engine/Source/BlizzardCore/Source/System/Debug/ErrorStack.h"
#include "Engine/Source/BlizzardCore/Source/System/Memory/MemoryStorm.h"
#include "Engine/Source/BlizzardCore/Source/System/Memory/TempBuffer.h"
#include "Engine/Source/BlizzardCore/Source/System/String/String.h"

// STATIC: 0x00AB909D
bool Blizzard::Debug::g_ErrorStackEnabled = true;

// VERIFY: f68d3fb4 manual
// OFFSET: 0x00427070
bool Blizzard::Debug::ErrorStackRecord::FindCategoryCode(int32_t category, int32_t code) {
    bool found = false;

    for (ErrorStackRecord* record = this; record; record = record->m_prev) {
        if (record->m_category == category && record->m_code == code) {
            found = true;
        }
    }

    return found;
}

// VERIFY: af269794 manual
// OFFSET: 0x004270A0
bool Blizzard::Debug::ErrorStackRecord::FindCategory(int32_t category) {
    bool found = false;

    for (ErrorStackRecord* record = this; record; record = record->m_prev) {
        if (record->m_category == category) {
            found = true;
        }
    }

    return found;
}

// VERIFY: 40137964 manual
// OFFSET: 0x004270D0
const char* Blizzard::Debug::ErrorStackAsText(ErrorStackRecord* stack) {
    return stack ? stack->m_flattened : "";
}

// VERIFY: e94d9434 manual
// OFFSET: 0x00432210
Blizzard::Debug::ErrorStackRecord* Blizzard::Debug::AddErrorToStack(ErrorStackRecord* stack, int32_t errCode, const char* errStr, int32_t category) {
    if (!g_ErrorStackEnabled) {
        return nullptr;
    }

    uint32_t strBytes = (errStr ? static_cast<uint32_t>(strlen(errStr)) : 0) + 1;
    int32_t level;
    const char* prevText;

    if (stack) {
        prevText = stack->m_flattened;
        level = stack->m_level + 1;
    } else {
        level = 0;
        prevText = "";
    }

    uint32_t bytes = strBytes + (prevText ? static_cast<uint32_t>(strlen(prevText)) : 0) + 100;
    Blizzard::Memory::TempBuffer<char, 2048> buffer(bytes);
    Blizzard::String::Format(buffer.m_data, bytes, "[%d] err=%d text=%s\n%s", level, errCode, errStr, prevText);
    uint32_t flatBytes = (buffer.m_data ? static_cast<uint32_t>(strlen(buffer.m_data)) : 0) + 1;

    ErrorStackRecord* record = static_cast<ErrorStackRecord*>(Blizzard::Memory::Allocate(sizeof(ErrorStackRecord)));
    record->m_code = errCode;
    record->m_prev = stack;
    record->m_level = level;
    record->m_category = category;
    record->m_str = static_cast<char*>(Blizzard::Memory::Allocate(strBytes));
    Blizzard::String::Copy(record->m_str, errStr, strBytes);
    record->m_flattened = static_cast<char*>(Blizzard::Memory::Allocate(flatBytes));
    Blizzard::String::Copy(record->m_flattened, buffer.m_data, flatBytes);

    ErrorStackRecord* head;

    if (stack) {
        ErrorStackRecord* prev = stack->m_prev;
        ErrorStackRecord swap;
        Blizzard::String::MemCopy(&swap, stack, sizeof(swap));
        Blizzard::String::MemCopy(stack, record, sizeof(swap));
        Blizzard::String::MemCopy(record, &swap, sizeof(swap));
        stack->m_prev = record;
        record->m_prev = prev;
        head = stack;
    } else {
        head = record;
    }

    if (head->m_level >= 16) {
        ErrorStackRecord* node = record;

        while (node->m_prev->m_prev) {
            node = node->m_prev;
        }

        Blizzard::Memory::Free(node->m_prev->m_flattened);
        Blizzard::Memory::Free(node->m_prev->m_str);
        Blizzard::Memory::Free(node->m_prev);
        node->m_prev = nullptr;
    }

    return head;
}

// VERIFY: dd8d9593 manual
// OFFSET: 0x004324E0
void Blizzard::Debug::FreeErrorStack(ErrorStackRecord* stack) {
    while (stack) {
        ErrorStackRecord* prev = stack->m_prev;
        Blizzard::Memory::Free(stack->m_flattened);
        Blizzard::Memory::Free(stack->m_str);
        Blizzard::Memory::Free(stack);
        stack = prev;
    }
}
