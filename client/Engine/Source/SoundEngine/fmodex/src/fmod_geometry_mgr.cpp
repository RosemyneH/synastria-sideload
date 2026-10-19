#include "fmod_settings.h"
#include "fmod_geometry_mgr.h"

#ifdef FMOD_SUPPORT_GEOMETRY

#include "fmod_memory.h"
#include "fmod_systemi.h"
#include "fmod_localcriticalsection.h"
#include "fmod_3d.h"

#if defined(FMOD_GEOMETRY_DEBUGGING)

// exported functions for debugging
void F_DECLSPEC F_DLLEXPORT geometryRenderTest(void (*renderBox)(float xMin, float xMax, float yMin, float yMax, float zMin, float zMax))
{
    if (FMOD::GeometryMgr::sThis)
        FMOD::GeometryMgr::sThis->renderTree(renderBox);
}

void F_DECLSPEC F_DLLEXPORT geometryLineTestTest(FMOD_VECTOR* start, FMOD_VECTOR* end, float* intensity)
{
    if (FMOD::GeometryMgr::sThis)
    {
        float directOcclusion = 0;
        float reverbOcclusion = 0;
        FMOD::GeometryMgr::sThis->lineTestAll(start, end, &directOcclusion, &reverbOcclusion);
        *intensity = 1.0f - directOcclusion;
    }
}

void F_DECLSPEC F_DLLEXPORT testLineTestForEachPolygon()
{
    // does a line test for ever polygon and make sure that it
    // intersects with it.
    // hasn't been made to work with transformations.
    if (FMOD::GeometryMgr::sThis)
    {
        FMOD::GeometryMgr::sThis->testLineTestForEachPolygon();
    }
}
#endif

namespace FMOD
{
    
/*
[
    [DESCRIPTION]

    [PARAMETERS]

    [RETURN_VALUE]

    [REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Macintosh, XBox, PlayStation 2, GameCube

    [SEE_ALSO]
]
*/

#if defined(_DEBUG) && defined(FMOD_GEOMETRY_DEBUGGING)
#define ASSERT(x) do { if (!(x)) __asm { __emit 0xF1 } } while (false)
#else
#define ASSERT(x)    
#endif
    
#if defined(FMOD_GEOMETRY_DEBUGGING)
GeometryMgr* GeometryMgr::sThis = 0;
#endif



/*
[
    [DESCRIPTION]

    [PARAMETERS]

    [RETURN_VALUE]

    [REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Macintosh, XBox, PlayStation 2, GameCube

    [SEE_ALSO]
]
*/
// VERIFY: b747c766 auto
// OFFSET: 0x008E54D0
GeometryMgr::GeometryMgr()
{
    mSystem = 0;
    mMainOctree = 0;
    mRefCount = 0;
    mFirstUpdateItem = 0;
    mMoved = true;
#if defined(FMOD_GEOMETRY_DEBUGGING)
    sThis = this;
#endif
}


/*
[
    [DESCRIPTION]

    [PARAMETERS]

    [RETURN_VALUE]

    [REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Macintosh, XBox, PlayStation 2, GameCube

    [SEE_ALSO]
]
*/
GeometryMgr::~GeometryMgr()
{
#if defined(FMOD_GEOMETRY_DEBUGGING)
    sThis = 0;
#endif
}


/*
[
    [DESCRIPTION]

    [PARAMETERS]

    [RETURN_VALUE]

    [REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Macintosh, XBox, PlayStation 2, GameCube

    [SEE_ALSO]
]
*/
FMOD_RESULT GeometryMgr::aquireMainOctree()
{
    mRefCount++;
    if (!mMainOctree)
    {
        mMainOctree = (Octree *)FMOD_Memory_Alloc(sizeof (Octree));
        if (!mMainOctree)
            return FMOD_ERR_MEMORY;
        new (mMainOctree) Octree(mWorldSize);
    }
    return FMOD_OK;
}


/*
[
    [DESCRIPTION]

    [PARAMETERS]

    [RETURN_VALUE]

    [REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Macintosh, XBox, PlayStation 2, GameCube

    [SEE_ALSO]
]
*/
void GeometryMgr::releaseMainOctree()
{
    mRefCount--;
    if (mRefCount <= 0)
    {
        mRefCount = 0;
        if (mMainOctree)
        {
            mMainOctree->~Octree();
            FMOD_Memory_Free(mMainOctree);
            mMainOctree = 0;
        }
    }
}


/*
[
    [DESCRIPTION]

    [PARAMETERS]

    [RETURN_VALUE]

    [REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Macintosh, XBox, PlayStation 2, GameCube

    [SEE_ALSO]
]
*/
// VERIFY: c4a87836 auto
// OFFSET: 0x008E54F0
bool GeometryMgr::mainOctreeLineTestCallback(OctreeNode* item, void* data)
{
    GeometryI::SpatialData* spatialdata = (GeometryI::SpatialData*)item;
    ASSERT(spatialdata->geometry);
    GeometryI::LineTestData* lineTestData = (GeometryI::LineTestData*)data;
    lineTestData->geometryI = spatialdata->geometry;

    return spatialdata->geometry->lineTest(lineTestData);
}

#ifdef _DEBUG
/*
[
    [DESCRIPTION]

    [PARAMETERS]

    [RETURN_VALUE]

    [REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Macintosh, XBox, PlayStation 2, GameCube

    [SEE_ALSO]
]
*/
Octree *GeometryMgr::mainOctree()
{
    ASSERT(mMainOctree);
    return mMainOctree;
}
#endif // _DEBUG


/*
[
    [DESCRIPTION]

    [PARAMETERS]

    [RETURN_VALUE]

    [REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Macintosh, XBox, PlayStation 2, GameCube

    [SEE_ALSO]
]
*/
FMOD_RESULT GeometryMgr::setWorldSize(float worldSize)
{
    if (mWorldSize == worldSize)
    {
        return FMOD_OK;
    }

    mWorldSize = worldSize;

    if (mMainOctree)
    {
        mMainOctree->setMaxSize(worldSize);
    }
    
    return FMOD_OK;
}


/*
[
    [DESCRIPTION]

    [PARAMETERS]

    [RETURN_VALUE]

    [REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Macintosh, XBox, PlayStation 2, GameCube

    [SEE_ALSO]
]
*/
// VERIFY: 10e77dd3 auto
// OFFSET: 0x008E5550
FMOD_RESULT GeometryMgr::lineTestAll(const FMOD_VECTOR* start, const FMOD_VECTOR* end, float* directOcclusion, float* reverbOcclusion)
{
    flushAll();
    
    GeometryI::LineTestData lineTestData;
    lineTestData.start = *start;
    lineTestData.end = *end;
    lineTestData.directTransmission = 1.0f;
    lineTestData.reverbTransmission = 1.0f;
    lineTestData.geometryI = 0;

    if (mMainOctree)
    {
        mainOctree()->testLine(mainOctreeLineTestCallback, &lineTestData, *start, *end);
    }

    *directOcclusion = 1.0f - lineTestData.directTransmission;
    *reverbOcclusion = 1.0f - lineTestData.reverbTransmission;
    
    return FMOD_OK;
}


/*
[
    [DESCRIPTION]

    [PARAMETERS]

    [RETURN_VALUE]

    [REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Macintosh, XBox, PlayStation 2, GameCube

    [SEE_ALSO]
]
*/
// VERIFY: 29a5ad33 manual
// OFFSET: 0x008E5510
FMOD_RESULT GeometryMgr::flushAll()
{    
    GeometryI* geometryI = mFirstUpdateItem;
    mFirstUpdateItem = 0;

    while (geometryI)
    {
        GeometryI* next = geometryI->mNextUpdateItem;
        
        geometryI->mNextUpdateItem = 0;
        ASSERT(geometryI->mToBeUpdated);
        geometryI->mToBeUpdated = false;
    
        geometryI->flush();       
        geometryI = next;
    }

    return FMOD_OK;
}

#if defined(FMOD_GEOMETRY_DEBUGGING)

/*
[
    [DESCRIPTION]

    [PARAMETERS]

    [RETURN_VALUE]

    [REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Macintosh, XBox, PlayStation 2, GameCube

    [SEE_ALSO]
]
*/
FMOD_RESULT GeometryMgr::renderTree(void (*renderBox)(float xMin, float xMax, float yMin, float yMax, float zMin, float zMax))
{
    mainOctree()->renderTree(renderBox);
    return FMOD_OK;
}

/*
[
    [DESCRIPTION]

    [PARAMETERS]

    [RETURN_VALUE]

    [REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Macintosh, XBox, PlayStation 2, GameCube

    [SEE_ALSO]
]
*/
FMOD_RESULT GeometryMgr::testLineTestForEachPolygon()
{
  	GeometryI* current = mSystem->mGeometryList;
	do
	{ 
	    current->testLineTestForEachPolygon();
		current = SAFE_CAST(GeometryI, current->getNext());
	} 
    while (current != mSystem->mGeometryList);

    return FMOD_OK;
}
#endif




/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]

	[SEE_ALSO]
]
*/



}

#endif // FMOD_SUPPORT_GEOMETRY
