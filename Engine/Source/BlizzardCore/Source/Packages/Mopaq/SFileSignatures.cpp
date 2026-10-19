#include <cctype>
#include <cstdint>
#include <cstring>
#include "Engine/Source/BlizzardCore/Source/Packages/Mopaq/SFileSignatures.h"
#include "Engine/Source/BlizzardCore/Source/Packages/Util/Hash.h"
#include "Engine/Source/BlizzardCore/Source/System/String/String.h"

// STATIC: 0x00B38338
System_SFile::SignatureTable System_SFile::s_signatures;

// VERIFY: 2da0c4e0 manual
// OFFSET: 0x0042ECC0
System_SFile::SignatureKey::SignatureKey(const char* name) {
    uint32_t pc = 2;
    uint32_t pb = 1;
    Blizzard::Util::hashlittle2(name, name ? strlen(name) : 0, &pc, &pb);

    uint64_t hash = pc + (static_cast<uint64_t>(pb) << 32);
    this->m_lo = static_cast<uint32_t>(hash) | 0x3F;
    this->m_hi = static_cast<uint32_t>(hash >> 32);
}

// VERIFY: 7f796dd8 manual
// OFFSET: 0x00437470
bool System_SFile::SignatureTable::GetSignature(const char* name, Blizzard::Util::Digest<16>* md5) {
    if (!md5) {
        return false;
    }

    char lower[260];
    Blizzard::String::Copy(lower, name, sizeof(lower));

    for (char* p = lower; *p; p++) {
        *p = static_cast<char>(tolower(*p));
    }

    SignatureKey key(lower);

    return this->Get(key, md5);
}

// VERIFY: 3eff0a7f manual
// OFFSET: 0x0043AB40
bool System_SFile::SignatureTable::SetSignature(const char* name, const Blizzard::Util::Digest<16>& md5) {
    char lower[260];
    Blizzard::String::Copy(lower, name, sizeof(lower));

    for (char* p = lower; *p; p++) {
        *p = static_cast<char>(tolower(*p));
    }

    SignatureKey key(lower);

    return this->Insert(key, md5);
}
