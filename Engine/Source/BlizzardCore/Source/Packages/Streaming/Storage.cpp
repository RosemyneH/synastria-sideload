#include <cstdint>
#include "Engine/Source/BlizzardCore/Source/Packages/Streaming/Storage.h"
#include "Engine/Source/BlizzardCore/Source/System/String/String.h"
#include "Engine/Source/BlizzardCore/Source/System/Using/Using.h"

// STATIC: 0x00AB93C8
static Blizzard::Using::Use s_useFile("Blizzard::File");

// STATIC: 0x00B3818C
int32_t Blizzard::Storage::s_variantIndex;

// STATIC: 0x00B38190
char Blizzard::Storage::s_variantName[260];

// VERIFY: 10d42236 auto
// OFFSET: 0x00432C70
void Blizzard::Storage::BuildVariantFileName(char* dest, uint32_t size, const char* name, const char* extension) {
    if (s_variantName[0]) {
        Blizzard::String::Format(dest, size, "%s.%s", name, s_variantName);
    } else {
        Blizzard::String::Copy(dest, name, size);
    }

    if (s_variantIndex > 0) {
        Blizzard::String::Format(dest, size, "%s.%d", dest, s_variantIndex);
    }

    if (*extension) {
        Blizzard::String::Format(dest, size, "%s.%s", dest, extension);
    }
}
