#ifndef WOW_SOURCE_GAME_VIOLENCELEVELS_H
#define WOW_SOURCE_GAME_VIOLENCELEVELS_H

#include <cstdint>

struct CVar;

extern int32_t s_violenceLevel;

bool ViolenceLevelChanged(CVar* var, const char* oldValue, const char* value, void* arg);
void ViolenceLevelsRegisterCVars();
int32_t ViolenceGetLevel();

#endif
