#include "MapWeather.h"
#include "Engine/Source/ConsoleBase/IConsole.h"
#include "Engine/Source/Storm/H/storm.h"
#include "WoW/Source/WorldClient/World.h"

// STATIC: 0x00CD852C
CVar* s_useShaders;

// VERIFY: 01bb4d2e auto
// OFFSET: 0x00784040
bool CVarHandler_weatherDensity(CVar* var, const char* oldValue, const char* value, void* arg) {
    switch (SStrToInt(value)) {
        case 0: {
            s_weatherDensity = 0.1f;
            break;
        }

        case 1: {
            s_weatherDensity = 0.33f;
            break;
        }

        case 2: {
            s_weatherDensity = 0.66f;
            break;
        }

        case 3: {
            s_weatherDensity = 1.0f;
            break;
        }

        default: {
            ConsoleWriteA("Value out of range (%d - %d)\n", DEFAULT_COLOR, 0, 3);
            return false;
        }
    }

    if (World::s_weather) {
        World::s_weather->m_densityChanged = 1;
    }

    return true;
}
