#ifndef WOW_SOURCE_OBJECT_OBJECTCLIENT_UNITMISSILETRAJECTORY_C_H
#define WOW_SOURCE_OBJECT_OBJECTCLIENT_UNITMISSILETRAJECTORY_C_H

#include <cstddef>
#include <cstdint>
#include "WoW/Source/Object/WGUID.h"

#include "Engine/Source/Tempest/C2Vector.h"

class C3Vector;
class CAaSphere;
class CGUnit_C;
class SpellMissileRec;
class CMissileCollision;

class CMissileCollisionHeap {
    public:
    /* 0000 */ uint32_t m_heapId;

    CMissileCollision* New();
};

class CMissileCollision {
    public:
    static CMissileCollisionHeap* s_pool;
    static CMissileCollision* s_head;
    static CMissileCollision* s_tail;

    static void InitSystem();
    static CMissileCollision* Alloc();
    static void RemoveUnitFromSystem(CGUnit_C* unit);
    static void MaybeAddUnitToSystem(CGUnit_C* unit);
    static int32_t MaybeAddUnitToWorkingList(WOWGUID guid, void* param);

    /* 0000 */ uint32_t m_memHandle;
    /* 0004 */ CGUnit_C* m_units [16];
    /* 0044 */ C2Vector m_start;
    /* 004C */ C2Vector m_end;
    /* 0054 */ C2Vector m_dir;
    /* 005C */ C2Vector m_perp;
    /* 0064 */ float m_length;
    /* 0068 */ float m_duration;
    /* 006C */ float m_speed;
    /* 0070 */ const SpellMissileRec* m_missile;
    /* 0074 */ CMissileCollision* m_prev;
    /* 0078 */ CMissileCollision* m_next;

    CMissileCollision();
    void Free();
    int32_t Init(const C3Vector* start, const C3Vector* end, float pitch, float speed, const SpellMissileRec* missile, CGUnit_C* caster);
    float ComputeSegmentImpactTime(const C3Vector* start, const C3Vector* end);
    float ComputeUnitRank(CGUnit_C* unit);
    void ComputeHitSpheres(CAaSphere* spheres);
};

#endif
