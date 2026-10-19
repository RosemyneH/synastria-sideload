#include <cstddef>
#include <cstdint>
#include "Engine/Source/Frame/CSimpleTranslationAnim.h"
#include "Engine/Source/Frame/CScriptObject.h"
#include "Engine/Source/Frame/CSimpleAnim.h"
#include "Engine/Source/Storm/H/storm.h"
#include "Engine/Source/lua-5.1/src/lauxlib.h"
#include "Engine/Source/Base/Coordinate.h"
#include "Engine/Source/Base/Status.h"
#include "Engine/Source/Frame/CScriptRegion.h"
#include "Engine/Source/Frame/CSimpleAnimGroup.h"
#include "Engine/Source/FrameXML/LoadXML.h"
#include "Engine/Source/FrameXML/XMLTree.h"

// STATIC: 0x00AC19EC
int32_t CSimpleTranslationAnim::s_metatable = LUA_NOREF;

// STATIC: 0x00B499F4
int32_t CSimpleTranslationAnim::s_objectType;

// STATIC: 0x00AC1390
const char* CSimpleTranslationAnim::s_objectTypeName = "Translation";

// INLINED
int32_t CSimpleTranslationAnim::GetObjectType() {
    if (!CSimpleTranslationAnim::s_objectType) {
        CSimpleTranslationAnim::s_objectType = ++FrameScript_Object::s_objectTypes;
    }

    return CSimpleTranslationAnim::s_objectType;
}

// VERIFY: 2d763647 auto
// OFFSET: 0x00498E10
bool CSimpleTranslationAnim::IsA(int32_t type) {
    return type == CSimpleTranslationAnim::s_objectType || type == CSimpleAnim::s_objectType || type == CScriptObject::s_objectType;
}

// VERIFY: 700dd37a auto
// OFFSET: 0x00498E40
bool CSimpleTranslationAnim::IsA(const char* typeName) {
    return !SStrCmpI(typeName, CSimpleTranslationAnim::s_objectTypeName, STORM_MAX_STR) || !SStrCmpI(typeName, CSimpleAnim::s_objectTypeName, STORM_MAX_STR) || !SStrCmpI(typeName, CScriptObject::s_objectTypeName, STORM_MAX_STR);
}

// VERIFY: d1d66ef7 auto
// OFFSET: 0x00498EA0
const char* CSimpleTranslationAnim::GetObjectTypeName() {
    return CSimpleTranslationAnim::s_objectTypeName;
}

// VERIFY: c14e6432 auto
// OFFSET: 0x00498EB0
int32_t CSimpleTranslationAnim::GetScriptMetaTable() {
    return CSimpleTranslationAnim::s_metatable;
}

// VERIFY: c0c727df auto
// OFFSET: 0x00497700
void CSimpleTranslationAnim::ApplyInverse(float progress) {
    this->Apply(-progress);
}

// VERIFY: d5cfbfce manual
// OFFSET: 0x00497FF0
void CSimpleTranslationAnim::SetOffset(const C2Vector& offset) {
    if (offset.y * offset.y + offset.x * offset.x <= 0.00000023841858f) {
        this->m_offset.x = 0.0f;
        this->m_offset.y = 0.0f;
    } else {
        this->m_offset = offset;
    }
}

// VERIFY: ced18174 manual
// OFFSET: 0x00498040
void CSimpleTranslationAnim::Apply(float progress) {
    CScriptRegion* region = this->GetRegionParent();

    if (region) {
        C2Vector translation;
        translation.x = this->m_offset.x * progress;
        translation.y = progress * this->m_offset.y;
        region->AddAnimTranslation(translation);
    }
}

// VERIFY: aedef79e manual
// OFFSET: 0x004997B0
CSimpleTranslationAnim::~CSimpleTranslationAnim() {
}

// VERIFY: cbd07d08 auto
// OFFSET: 0x00499C50
CSimpleTranslationAnim::CSimpleTranslationAnim(CSimpleAnimGroup* group) : CSimpleAnim(group) {
}

// VERIFY: cb4fa63d auto
// OFFSET: 0x0049BB00
void CSimpleTranslationAnim::LoadXML(XMLNode* node, CStatus* status) {
    CSimpleAnim::LoadXML(node, status);

    C2Vector offset;

    const char* offsetX = node->GetAttributeByName("offsetX");

    if (offsetX && *offsetX) {
        float value = SStrToFloat(offsetX);
        offset.x = NDCToDDCWidth(value / (CoordinateGetAspectCompensation() * 1024.0f));
    }

    const char* offsetY = node->GetAttributeByName("offsetY");

    if (offsetY && *offsetY) {
        float value = SStrToFloat(offsetY);
        offset.y = NDCToDDCWidth(value / (CoordinateGetAspectCompensation() * 1024.0f));
    }

    this->SetOffset(offset);

    for (XMLNode* child = node->m_child; child; child = child->m_next) {
        if (!SStrCmpI(child->m_name.m_str, "Scripts", STORM_MAX_STR)) {
            this->LoadXML_Scripts(child, status);
        }
    }
}
