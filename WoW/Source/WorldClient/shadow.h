#ifndef WOW_SOURCE_WORLDCLIENT_SHADOW_H
#define WOW_SOURCE_WORLDCLIENT_SHADOW_H

#include <cstdint>

struct CVar;

extern CVar* s_cvExtShadowQuality;
extern CVar* s_cvShadowLOD;
extern int32_t s_shadowLOD;

bool ShadowLODCallback(CVar* var, const char* oldValue, const char* value, void* arg);
void ShadowDestroy();
void ShadowInit();
void ShadowSetLOD(int32_t lod);

#endif
