#include <cstdint>
#include <cstdlib>
#include "Engine/Source/FFX/FFX.h"
#include "Engine/Source/ConsoleBase/ConsoleVar.h"
#include "Engine/Source/ConsoleBase/IConsole.h"

// STATIC: 0x00D4576C
CVar* FFX::s_cvarMultisample;

// STATIC: 0x00D45770
CVar* FFX::s_cvarFFXRectangle;

// STATIC: 0x00D45774
CVar* FFX::s_cvarFFX;

// VERIFY: d767d877 manual
// OFFSET: 0x008C02A0
bool FFX::CVarCallback(CVar* var, const char* oldValue, const char* value, void* arg) {
    if (atol(value)) {
        ConsoleWrite("enabled", DEFAULT_COLOR);
    } else {
        ConsoleWrite("disabled", DEFAULT_COLOR);
    }

    return true;
}
