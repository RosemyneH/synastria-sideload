#ifndef ENGINE_SOURCE_BLIZZARDCORE_SOURCE_PACKAGES_MOPAQ_SFILESIGNATURES_H
#define ENGINE_SOURCE_BLIZZARDCORE_SOURCE_PACKAGES_MOPAQ_SFILESIGNATURES_H

#include <cstdint>
#include "Engine/Source/BlizzardCore/Include/Blizzard/Util/Digest.h"
#include "Engine/Source/BlizzardCore/Include/Blizzard/Util/HashMap.h"

namespace System_SFile {

struct SignatureKey {
    /* 0000 */ uint32_t m_lo;
    /* 0004 */ uint32_t m_hi;

    SignatureKey(const char* name);
};

class SignatureTable : public Blizzard::Util::HashMap<SignatureKey, Blizzard::Util::Digest<16>> {
    public:
    bool GetSignature(const char* name, Blizzard::Util::Digest<16>* md5);
    bool SetSignature(const char* name, const Blizzard::Util::Digest<16>& md5);
};

extern SignatureTable s_signatures;

}

#endif
