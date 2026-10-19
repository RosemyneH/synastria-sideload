#ifndef WOW_SOURCE_CONSOLEDEVICE_CONSOLEDETECT_H
#define WOW_SOURCE_CONSOLEDEVICE_CONSOLEDETECT_H

#include <cstdint>
#include "WoW/Source/Types.h"

void ConsoleDetectDetectHardware(Hardware& hardware, bool& changed);
void ConsoleDetectSaveHardware(const Hardware& hardware, bool& changed);
void ConsoleDetectSetDefaults(DefaultSettings& defaults, const Hardware& hardware);
void ConsoleDetectSetDefaultsFormat(DefaultSettings& defaults, const Hardware& hardware);

#endif
