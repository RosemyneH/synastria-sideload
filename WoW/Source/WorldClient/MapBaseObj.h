#ifndef WOW_SOURCE_WORLDCLIENT_MAPBASEOBJ_H
#define WOW_SOURCE_WORLDCLIENT_MAPBASEOBJ_H

#include <cstddef>
#include <cstdint>
#include "Engine/Source/Storm/H/stpl.h"
#include "WoW/Source/WorldClient/CMapBaseObjLink.h"
#include "WoW/Source/WorldClient/Types.h"

class CM2Lighting;
class CMapBaseObj;

class CMapBaseObj {
    public:
    /* 0004 */ int32_t m_objectIndex;
    /* 0008 */ uint16_t m_type;
    /* 000A */ uint16_t m_refCount;
    /* 000C */ MapObjFlag m_flags;
    /* 0010 */ TSLink<CMapBaseObj> m_lameAssLink;
    /* 0018 */ TSExplicitList<CMapBaseObjLink, offsetof(CMapBaseObjLink, m_ownerLink)> m_parentLinkList;

    virtual ~CMapBaseObj();
    virtual void SelectLights(CM2Lighting* lighting);
    virtual void SelectUnderwater(CM2Lighting* lighting);

    CMapBaseObj();
};

#endif
