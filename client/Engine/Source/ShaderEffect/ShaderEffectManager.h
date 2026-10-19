#ifndef ENGINE_SOURCE_SHADEREFFECT_SHADEREFFECTMANAGER_H
#define ENGINE_SOURCE_SHADEREFFECT_SHADEREFFECTMANAGER_H

#include <cstddef>
#include <cstdint>
#include "Engine/Source/ShaderEffect/ShaderEffect.h"
#include "Engine/Source/ShaderEffect/ShaderEffectParser.h"
#include "Engine/Source/Storm/H/stpl.h"

class CShaderEffectManager {
    public:
    static TSHashTable<CShaderEffect, HASHKEY_STRI> m_ShaderList;

    static void AddEffectFile(const char* fileName);
    static CShaderEffect* CreateEffect(const char* name);
    static void DestroyAllEffects();
    static CShaderEffect* GetEffect(const char* name);
    static void ParseEffectCallback(const CShaderEffectParser::ParsedEffect& effect, void* param);
};

#endif
