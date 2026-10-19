#ifndef _FMOD_GEOMETRYMGR_H
#define _FMOD_GEOMETRYMGR_H

#include "fmod_settings.h"

#ifdef FMOD_SUPPORT_GEOMETRY

#include "fmod_octree.h"
#include "fmod_geometryi.h"
#include "fmod_thread.h"
#include "fmod_channeli.h"

#ifndef _FMOD_MEMORYTRACKER_H
#include "fmod_memorytracker.h"
#endif

namespace FMOD
{
    class GeometryI;
    class SystemI;
    class GeometryMgr;
   

    class GeometryMgr
    {
        DECLARE_MEMORYTRACKER

      public:       
        friend class GeometryI;
        friend class ChannelI;
       
        SystemI             *mSystem;
        bool                 mMoved;
#if defined(FMOD_GEOMETRY_DEBUGGING)
        static GeometryMgr  *sThis;
#endif
       
        GeometryMgr();
        ~GeometryMgr();
       
        FMOD_RESULT  aquireMainOctree          ();
        void         releaseMainOctree         ();
#ifdef _DEBUG
        Octree      *mainOctree                ();
#else
        Octree      *mainOctree                ()  { return mMainOctree; }
#endif
        FMOD_RESULT  setWorldSize              (float worldSize);    
        float        getWorldSize              ()  { return mWorldSize; }        
        FMOD_RESULT  lineTestAll               (const FMOD_VECTOR* start, const FMOD_VECTOR* end, float* directOcclusion, float* reverbOcclusion);
         
#if defined(FMOD_GEOMETRY_DEBUGGING)
        FMOD_RESULT  renderTree                (void (*renderBox)(float xMin, float xMax, float yMin, float yMax, float zMin, float zMax));
        FMOD_RESULT  testLineTestForEachPolygon();
#endif
       
         static bool mainOctreeLineTestCallback (OctreeNode* item, void* data);
       
       private:
         FMOD_RESULT  flushAll                  ();

         Octree                    *mMainOctree;
         int                        mRefCount;
         GeometryI                 *mFirstUpdateItem;
         float                      mWorldSize;           
    };    
}

#endif

#endif

