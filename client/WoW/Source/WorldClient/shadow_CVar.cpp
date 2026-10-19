#include "shadow.h"
#include <cstdio>
#include "Engine/Source/ConsoleBase/ConsoleVar.h"
#include "Engine/Source/ConsoleBase/IConsole.h"

// STATIC: 0x00D38048
CVar* s_cvExtShadowQuality;

// STATIC: 0x00D3804C
CVar* s_cvShadowLOD;

// VERIFY: 1c2d78bb auto
// OFFSET: 0x007E3A20
bool ShadowLODCallback(CVar* var, const char* oldValue, const char* value, void* arg) {
    uint32_t lod;
    sscanf(value, "%d", &lod);

    if (lod <= 1) {
        ShadowSetLOD(lod);

        char buffer[256];
        sprintf(buffer, "Shadow LOD set to %d", s_shadowLOD);
        ConsoleWrite(buffer, DEFAULT_COLOR);

        return true;
    }

    ConsoleWrite("Shadow LOD must be in the range (0, 1)", DEFAULT_COLOR);

    return false;
}
