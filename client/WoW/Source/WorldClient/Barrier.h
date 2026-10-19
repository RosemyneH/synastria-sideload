#ifndef WOW_SOURCE_WORLDCLIENT_BARRIER_H
#define WOW_SOURCE_WORLDCLIENT_BARRIER_H

#include <cstddef>
#include <cstdint>
#include "Engine/Source/Tempest/C3Vector.h"
#include "Engine/Source/Tempest/C4Plane.h"
#include "Engine/Source/Tempest/CImVector.h"
#include "Engine/Source/Storm/H/stpl.h"

class C2Vector;
class C44Matrix;
class CAaBox;
class CTexture;
class CM2Model;
class CMapChunk;
class CMapObjDefGroup;

class CBarrier_Contact {
    public:
    /* 0000 */ C3Vector m_point;
    /* 000C */ float m_strength;
    /* 0010 */ C4Plane m_plane;
};

class CGxVertexPCT0T1 {
    public:
    /* 0000 */ C3Vector m_pos;
    /* 000C */ CImVector m_color;
    /* 0010 */ float m_t0x;
    /* 0014 */ float m_t0y;
    /* 0018 */ float m_t1x;
    /* 001C */ float m_t1y;

    CGxVertexPCT0T1();
};

class CBarrier {
    public:
    static void SetViewerPos(const C3Vector& position);
    static CM2Model* s_sparkles[4];
    static CTexture* s_mask;
    static CTexture* s_pattern;

    static void Destroy();
    static void Initialize();

    /* 0000 */ int32_t m_mode;
    /* 0004 */ TSGrowableArray<CGxVertexPCT0T1> m_verts;
    /* 0014 */ TSGrowableArray<CBarrier_Contact> m_contacts;
    /* 0024 */ TSGrowableArray<CMapChunk*> m_chunks;
    /* 0034 */ TSGrowableArray<CMapObjDefGroup*> m_groups;
    /* 0044 */ C3Vector m_viewerPos;

    void AddAxisAlignedWall(const C2Vector& start, const C2Vector& end, float radius);
    void AddBarrier(const CAaBox& box, float radius);
    void AddBarrierConvexPoly(C3Vector* verts, uint32_t count, float radius, C44Matrix* mat, int32_t flip, int32_t oneSided);
    void AddBarrierMapObjDefGroup(CMapObjDefGroup* mapObjDefGroup, float radius);
    void Render(const C3Vector& viewerPos, float elapsedSec);
};

#endif
