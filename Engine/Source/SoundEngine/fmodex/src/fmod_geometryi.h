#ifndef _FMOD_GEOMETRYI_H
#define _FMOD_GEOMETRYI_H

#include "fmod_settings.h"

#ifdef FMOD_SUPPORT_GEOMETRY

#include "fmod_linkedlist.h"
#include "fmod_octree.h"

#include "fmod.hpp"

#ifndef _FMOD_MEMORYTRACKER_H
#include "fmod_memorytracker.h"
#endif

namespace FMOD
{    
    class GeometryMgr;
    enum 
    {
        FMOD_POLYGON_NUM_VERTICES_MASK = 0xffff,
        FMOD_POLYGON_FLAG_DOUBLE_SIDED = 0x10000,
        FMOD_POLYGON_FLAG_INVALID = 0x20000,
    };
    struct FMOD_POLYGON
    {
        OctreeNode      node;
        OctreeNode      nodeInternal;
        float           distance;
        FMOD_VECTOR     normal;
        float           directOcclusion;
        float           reverbOcclusion;
        int             flags;
        FMOD_VECTOR     vertices;
    };

    class GeometryI : public LinkedListNode
    {
        DECLARE_MEMORYTRACKER

      private:
        struct SpatialData
        {
            OctreeNode  octreeNode;
            OctreeNode  octreeInternalNode;
            GeometryI  *geometry;
        };          
        struct LineTestData
        {
            FMOD_VECTOR start;
            FMOD_VECTOR end;
            float       directTransmission; // 1.0f - directOcclusion
            float       reverbTransmission; // 1.0f - reverbOcclusion
            GeometryI  *geometryI;
        };

        friend class GeometryMgr;

        GeometryMgr    *mGeometryMgr;
        int             mMaxNumVertices;
        int             mNumVertices;
        int             mMaxNumPolygons;
        int             mNumPolygons;
        int            *mPolygonOffsets;
        int             mPolygonDataPos;
        unsigned char  *mPolygonData;
        void           *mUserData;
        OctreeNode     *mPolygonUpdateList;
        FMOD_AABB       mAABB;

        bool            mActive;
        FMOD_VECTOR     mForward;
        FMOD_VECTOR     mUp;
        FMOD_VECTOR     mPosition;
        FMOD_VECTOR     mScale;
        float           mMatrix[3][4];
        float           mInvMatrix[3][4];
        
        SpatialData    *mSpatialData;
        Octree          mOctree;
        GeometryI      *mNextUpdateItem;
        bool            mToBeUpdated;

        void            calculateMatrix();
        void            updateSpatialData();
        void            setToBeUpdated();
        
      public:

        GeometryI(GeometryMgr* geometryMgr);

        FMOD_RESULT release                 ();       
        FMOD_RESULT alloc                   (int maxNumPolygons, int maxNumVertices);
    
        FMOD_RESULT addPolygon              (float directOcclusion, float reverbOcclusion, bool doubleSided, int numVertices, const FMOD_VECTOR *vertices, int *polygonIndex); 

        FMOD_RESULT getNumPolygons          (int *numPolygons); 
        FMOD_RESULT getMaxPolygons          (int *maxPolygons, int *maxVertices);
        FMOD_RESULT getPolygonNumVertices   (int polygonIndex, int *numVertices);
        FMOD_RESULT setPolygonVertex        (int polygonIndex, int vertexIndex, const FMOD_VECTOR *vertex); 
        FMOD_RESULT getPolygonVertex        (int polygonIndex, int vertexIndex, FMOD_VECTOR *vertex);
        FMOD_RESULT setPolygonAttributes    (int polygonIndex, float directOcclusion, float reverbOcclusion, bool doubleSided); 
        FMOD_RESULT getPolygonAttributes    (int polygonIndex, float *directOcclusion, float *reverbOcclusion, bool *doubleSided); 
        FMOD_RESULT flush                   ();

        FMOD_RESULT setActive               (bool active);
        FMOD_RESULT getActive               (bool *active);
        FMOD_RESULT setRotation             (const FMOD_VECTOR *forward, const FMOD_VECTOR *up);
        FMOD_RESULT getRotation             (FMOD_VECTOR *forward, FMOD_VECTOR *up);
        FMOD_RESULT setPosition             (const FMOD_VECTOR *position);
        FMOD_RESULT getPosition             (FMOD_VECTOR *position);
        FMOD_RESULT setScale                (const FMOD_VECTOR *scale);
        FMOD_RESULT getScale                (FMOD_VECTOR *scale);

        FMOD_RESULT save                    (void *data, int *dataSize);
        FMOD_RESULT load                    (const void *data, int dataSize);
        static FMOD_RESULT saveData         (void* fileData, int dataSize, int* fileDataIndex, void* liveData, int liveDataSize);
        static FMOD_RESULT loadData         (void* fileData, int dataSize, int* fileDataIndex, void* liveData, int liveDataSize);
        static FMOD_RESULT countData        (void* fileData, int dataSize, int* fileDataIndex, void* liveData, int liveDataSize);
        FMOD_RESULT serialiser              (void *data, int *dataSize, bool bWrite, bool bRead, FMOD_RESULT (*serialiseData)(void* fileData, int dataSize, int* fileDataIndex, void* liveData, int liveDataSize));
        
        // Userdata set/get.
        FMOD_RESULT F_API setUserData       (void *userdata);
        FMOD_RESULT F_API getUserData       (void **userdata);
        


        bool        lineTest                (LineTestData* lineTestData);

        FMOD_RESULT setWorldSize            (float worldSize);
        void        removeFromTree          ();
     
#if defined(FMOD_GEOMETRY_DEBUGGING)
        void testLineTestForEachPolygon();
#endif
    
        static FMOD_RESULT validate(Geometry *geometry, GeometryI **geometryi);    
        static bool octreeLineTestCallback(OctreeNode* item, void* data);
    };
}


#else

#include "fmod.hpp"

namespace FMOD
{    
    class GeometryI
    {        
      public:

        GeometryI() {}
        ~GeometryI() {}

        FMOD_RESULT F_API addPolygon            (float directOcclusion, float reverbOcclusion, bool doubleSided, int numVertices, const FMOD_VECTOR *vertices, int *polygonIndex) { return FMOD_ERR_UNIMPLEMENTED; } 

        FMOD_RESULT F_API getNumPolygons        (int *numPolygons) { return FMOD_ERR_UNIMPLEMENTED; }
        FMOD_RESULT F_API getMaxNumPolygons     (int *maxNumPolygons) { return FMOD_ERR_UNIMPLEMENTED; }
        FMOD_RESULT F_API getPolygonNumVertices (int polygonIndex, int *numVertices) { return FMOD_ERR_UNIMPLEMENTED; }
        FMOD_RESULT F_API setPolygonVertex      (int polygonIndex, int vertexIndex, const FMOD_VECTOR *vertex) { return FMOD_ERR_UNIMPLEMENTED; }
        FMOD_RESULT F_API getPolygonVertex      (int polygonIndex, int vertexIndex, FMOD_VECTOR *vertex) { return FMOD_ERR_UNIMPLEMENTED; }
        FMOD_RESULT F_API setPolygonAttributes  (int polygonIndex, float directOcclusion, float reverbOcclusion, bool doubleSided) { return FMOD_ERR_UNIMPLEMENTED; }
        FMOD_RESULT F_API getPolygonAttributes  (int polygonIndex, float *directOcclusion, float *reverbOcclusion, bool *doubleSided) { return FMOD_ERR_UNIMPLEMENTED; }
        FMOD_RESULT F_API flush                 () { return FMOD_ERR_UNIMPLEMENTED; }

        FMOD_RESULT F_API setRotation           (const FMOD_VECTOR *forward, const FMOD_VECTOR *up) { return FMOD_ERR_UNIMPLEMENTED; }
        FMOD_RESULT F_API getRotation           (FMOD_VECTOR *forward, FMOD_VECTOR *up) { return FMOD_ERR_UNIMPLEMENTED; }
        FMOD_RESULT F_API setPosition           (const FMOD_VECTOR *position) { return FMOD_ERR_UNIMPLEMENTED; }
        FMOD_RESULT F_API getPosition           (FMOD_VECTOR *position) { return FMOD_ERR_UNIMPLEMENTED; }
        FMOD_RESULT F_API setScale              (const FMOD_VECTOR *scale) { return FMOD_ERR_UNIMPLEMENTED; }
        FMOD_RESULT F_API getScale              (FMOD_VECTOR *scale) { return FMOD_ERR_UNIMPLEMENTED; }

    
        static FMOD_RESULT validate(Geometry *geometry, GeometryI **geometryi) { return FMOD_ERR_UNIMPLEMENTED; }

    };
}

#endif


#endif

