#include "fmod_geometryi.h"

#ifdef FMOD_SUPPORT_GEOMETRY

#include "fmod_3d.h"
#include "fmod_memory.h"
#include "fmod_geometry_mgr.h"
#include "fmod_systemi.h"
#include "fmod_localcriticalsection.h"

#include <string.h>

namespace FMOD
{

#if defined(_DEBUG) && defined(FMOD_GEOMETRY_DEBUGGING)
#define ASSERT(x) do { if (!(x)) __asm { int 3 } } while (false)
#else
#define ASSERT(x)    
#endif

// if the sound transsmision drop this low then
// don't bother doing any more ray testing
const float MIN_TRANSMISSION = 0.05f;

static inline float MAX(float  x, float  y) { return x > y ? x : y; }
static inline float MIN(float  x, float  y) { return x < y ? x : y; }


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
static inline void cross(const FMOD_VECTOR& a, const FMOD_VECTOR& b, FMOD_VECTOR& result)
{
    result.x = a.y * b.z - a.z * b.y;
    result.y = a.z * b.x - a.x * b.z;
    result.z = a.x * b.y - a.y * b.x;
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
static void matrixMult(const float matrix[3][4], const FMOD_VECTOR *src, FMOD_VECTOR *dst)
{
    dst->x = matrix[0][0] * src->x + matrix[0][1] * src->y + matrix[0][2] * src->z;
    dst->y = matrix[1][0] * src->x + matrix[1][1] * src->y + matrix[1][2] * src->z;
    dst->z = matrix[2][0] * src->x + matrix[2][1] * src->y + matrix[2][2] * src->z;
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
void GeometryI::calculateMatrix()
{
    mMatrix[1][0] = mUp.x * mScale.y;
    mMatrix[1][1] = mUp.y * mScale.y;
    mMatrix[1][2] = mUp.z * mScale.y;
    mMatrix[2][0] = mForward.x * mScale.z;
    mMatrix[2][1] = mForward.y * mScale.z;
    mMatrix[2][2] = mForward.z * mScale.z;
    mMatrix[0][0] = (mUp.y * mForward.z - mUp.z * mForward.y) * mScale.x;
    mMatrix[0][1] = (mUp.z * mForward.x - mUp.x * mForward.z) * mScale.x;
    mMatrix[0][2] = (mUp.x * mForward.y - mUp.y * mForward.x) * mScale.x;

    mInvMatrix[0][1] = mUp.x / mScale.y;
    mInvMatrix[1][1] = mUp.y / mScale.y;
    mInvMatrix[2][1] = mUp.z / mScale.y;
    mInvMatrix[0][2] = mForward.x / mScale.z;
    mInvMatrix[1][2] = mForward.y / mScale.z;
    mInvMatrix[2][2] = mForward.z / mScale.z;
    mInvMatrix[0][0] = (mUp.y * mForward.z - mUp.z * mForward.y) / mScale.x;
    mInvMatrix[1][0] = (mUp.z * mForward.x - mUp.x * mForward.z) / mScale.x;
    mInvMatrix[2][0] = (mUp.x * mForward.y - mUp.y * mForward.x) / mScale.x;
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
// VERIFY: c0ddaa05 auto
// OFFSET: 0x00919D30
void GeometryI::updateSpatialData()
{
    // find world aabb
    FMOD_VECTOR center;
    center.x = (mAABB.xMax + mAABB.xMin) * 0.5f;
    center.y = (mAABB.yMax + mAABB.yMin) * 0.5f;
    center.z = (mAABB.zMax + mAABB.zMin) * 0.5f;
    FMOD_VECTOR newCenter;
    matrixMult(mMatrix, &center, &newCenter);
    newCenter.x += mPosition.x;
    newCenter.y += mPosition.y;
    newCenter.z += mPosition.z;


    float xExtent = (mAABB.xMax - mAABB.xMin) * 0.5f;
    float yExtent = (mAABB.yMax - mAABB.yMin) * 0.5f;
    float zExtent = (mAABB.zMax - mAABB.zMin) * 0.5f;

    float xNewExtent = (float)fabs(mMatrix[0][0]) * xExtent + (float)fabs(mMatrix[1][0]) * yExtent + (float)fabs(mMatrix[2][0]) * zExtent;
    float yNewExtent = (float)fabs(mMatrix[0][1]) * xExtent + (float)fabs(mMatrix[1][1]) * yExtent + (float)fabs(mMatrix[2][1]) * zExtent;
    float zNewExtent = (float)fabs(mMatrix[0][2]) * xExtent + (float)fabs(mMatrix[1][2]) * yExtent + (float)fabs(mMatrix[2][2]) * zExtent;


    ASSERT(mSpatialData);
    mSpatialData->octreeNode.aabb.xMax = newCenter.x + xNewExtent;
    mSpatialData->octreeNode.aabb.xMin = newCenter.x - xNewExtent;
    mSpatialData->octreeNode.aabb.yMax = newCenter.y + yNewExtent;
    mSpatialData->octreeNode.aabb.yMin = newCenter.y - yNewExtent;
    mSpatialData->octreeNode.aabb.zMax = newCenter.z + zNewExtent;
    mSpatialData->octreeNode.aabb.zMin = newCenter.z - zNewExtent;

    if (mActive)
    {
        mGeometryMgr->mainOctree()->updateItem(&mSpatialData->octreeNode);
    }
    else
    {
        mGeometryMgr->mainOctree()->deleteItem(&mSpatialData->octreeNode);
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
void GeometryI::setToBeUpdated()
{
    mGeometryMgr->mMoved = true;

    if (!mToBeUpdated)
    {        
        mToBeUpdated = true;
        mNextUpdateItem = mGeometryMgr->mFirstUpdateItem;
        mGeometryMgr->mFirstUpdateItem = this;
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
// VERIFY: dc27e90c manual
// OFFSET: 0x00919EC0
bool GeometryI::octreeLineTestCallback(OctreeNode* item, void* data)
{
    LineTestData        *lineTestData = (LineTestData*)data;

    ASSERT(lineTestData);
    ASSERT(lineTestData->geometryI);

    const FMOD_VECTOR&   a = lineTestData->start;
    const FMOD_VECTOR&   b = lineTestData->end;

    FMOD_POLYGON& polygon = *(FMOD_POLYGON*)item;

    float dotA = FMOD_Vector_DotProduct(&polygon.normal, &a) - polygon.distance;
    float dotB = FMOD_Vector_DotProduct(&polygon.normal, &b) - polygon.distance;

    if (dotA >= 0.0f && dotB >= 0.0f || dotA <= 0.0f && dotB <= 0.0f)
    {
        return true; // no collision
    }

    if (dotA > 0.0f && (polygon.flags & FMOD_POLYGON_FLAG_DOUBLE_SIDED) == 0)
    {
        return true; // no collision
    }

    // find point on plane
    float time = dotA / (dotA - dotB);

    FMOD_VECTOR collisionPosition;

    collisionPosition.x = a.x + (b.x - a.x) * time;
    collisionPosition.y = a.y + (b.y - a.y) * time;
    collisionPosition.z = a.z + (b.z - a.z) * time;

    FMOD_VECTOR* vertices = &polygon.vertices;

    // find if point lies inside all edges of polygon
    int side;
    for (side = 0; side < (polygon.flags & FMOD_POLYGON_NUM_VERTICES_MASK); side++)
    {
        FMOD_VECTOR diff;
        FMOD_VECTOR edgeVector;
        int next = side + 1;

        if (next >= (polygon.flags & FMOD_POLYGON_NUM_VERTICES_MASK))
        {
            next = 0;
        }

        diff.x = vertices[next].x - vertices[side].x;
        diff.y = vertices[next].y - vertices[side].y;
        diff.z = vertices[next].z - vertices[side].z;

        cross(diff, polygon.normal, edgeVector);

        float dot = 
            edgeVector.x * (collisionPosition.x - vertices[side].x) +
            edgeVector.y * (collisionPosition.y - vertices[side].y) +
            edgeVector.z * (collisionPosition.z - vertices[side].z);

        if (dot > 0.0f)
        {
            break; // point is outside polygon so no interestection occured
        }
    }
    if (side == (polygon.flags & FMOD_POLYGON_NUM_VERTICES_MASK))
    {
        lineTestData->directTransmission *= (1.0f - polygon.directOcclusion);
        lineTestData->reverbTransmission *= (1.0f - polygon.reverbOcclusion);

#if defined(FMOD_GEOMETRY_DEBUGGING)
        // for function testLineTestForEachPolygon to see if this
        // polygon has actually been intersected with
        polygon.flags &= ~0x80000000;
#else
        if (lineTestData->directTransmission < MIN_TRANSMISSION &&
            lineTestData->reverbTransmission < MIN_TRANSMISSION)
        {
            // sounds level has dropped too low so stop doing line testing
            return false;
        }    
#endif
    } 

    return true;
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
GeometryI::GeometryI(GeometryMgr* geometryMgr) : LinkedListNode(), mOctree(geometryMgr->getWorldSize())
{
    mGeometryMgr = geometryMgr;
    mMaxNumVertices = 0;
    mNumVertices = 0;
    mMaxNumPolygons = 0;
    mNumPolygons = 0;
    mPolygonOffsets = 0;
    mPolygonDataPos = 0;
    mPolygonData = 0;

    mForward.x = 0.0f;
    mForward.y = 0.0f;
    mForward.z = 1.0f;
    mUp.x = 0.0f;
    mUp.y = 1.0f;
    mUp.z = 0.0f;
    mPosition.x = 0.0f;
    mPosition.y = 0.0f;
    mPosition.z = 0.0f;
    mScale.x = 1.0f;
    mScale.y = 1.0f;
    mScale.z = 1.0f;
    calculateMatrix();

    mPolygonUpdateList = 0;
    mNextUpdateItem = 0;
    mToBeUpdated = false;
    mActive = true;    
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
FMOD_RESULT GeometryI::release()
{      
   // mGeometryMgr->flushAll();

    // find out if we are in the "to be updated" list and remove us.
    // Other wise we will try to update a deleted object
    GeometryI* geometryI = mGeometryMgr->mFirstUpdateItem;
    GeometryI* prevGeometryI = 0;
    while (geometryI)
    {
        if (geometryI == this)
        {
            if (prevGeometryI)
            {
                prevGeometryI->mNextUpdateItem = geometryI->mNextUpdateItem;
            }
            else
            {
                mGeometryMgr->mFirstUpdateItem = geometryI->mNextUpdateItem;
            }
            break;
        }
        prevGeometryI = geometryI;    
        geometryI = geometryI->mNextUpdateItem;
    }


    if (mSpatialData)
    {
        mGeometryMgr->mainOctree()->deleteItem(&mSpatialData->octreeNode);
        mGeometryMgr->mainOctree()->removeInternalNode(&mSpatialData->octreeInternalNode);
        mGeometryMgr->releaseMainOctree();
        FMOD_Memory_Free(mSpatialData);
        mSpatialData = 0;
    }
    if (mPolygonData)
    {
        FMOD_Memory_Free(mPolygonData);
        mPolygonData = 0;
    }
    if (mPolygonOffsets)
    {
        FMOD_Memory_Free(mPolygonOffsets);
        mPolygonOffsets = 0;
    }

    mGeometryMgr->mMoved = true;    // Make it reset the voices back to unoccluded.
    mGeometryMgr->mSystem->update();

    if (mGeometryMgr->mSystem->mGeometryList == this)
    {
        mGeometryMgr->mSystem->mGeometryList = (GeometryI*)getNext();
        if (mGeometryMgr->mSystem->mGeometryList == this)
        {
            mGeometryMgr->mSystem->mGeometryList = 0;
        }
    }
    removeNode();

    FMOD_Memory_Free(this);

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
FMOD_RESULT GeometryI::alloc(int maxNumPolygons, int maxNumVertices)
{    
    FMOD_RESULT result;

    if (mPolygonData || mPolygonOffsets )
    {
        return FMOD_ERR_INTERNAL;
    }

    mMaxNumVertices = maxNumVertices;
    mNumVertices = 0;
    mMaxNumPolygons = maxNumPolygons;
    mNumPolygons = 0;
    mPolygonOffsets = (int*)FMOD_Memory_Alloc(mMaxNumVertices * sizeof (int));
    if (!mPolygonOffsets)
    {
        return FMOD_ERR_MEMORY;
    }
    mPolygonDataPos = 0;
    int polygonDataSize = (sizeof(FMOD_POLYGON) - sizeof (FMOD_VECTOR)) * maxNumPolygons;
    polygonDataSize += sizeof (FMOD_VECTOR) * maxNumVertices;

    mPolygonData = (unsigned char*)FMOD_Memory_Alloc(polygonDataSize);
    if (!mPolygonData)
    {
        return FMOD_ERR_MEMORY;
    }

    result = mGeometryMgr->aquireMainOctree();
    if (result != FMOD_OK)
    {
        return result;
    }

    mSpatialData = (SpatialData *)FMOD_Memory_Alloc(sizeof (SpatialData));
    if (!mSpatialData)
    {
        return FMOD_ERR_MEMORY;
    }

    FMOD_memset(mSpatialData, 0, sizeof (SpatialData));
    mSpatialData->geometry = this;
    
    mGeometryMgr->mainOctree()->addInternalNode(&mSpatialData->octreeInternalNode);

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
FMOD_RESULT GeometryI::addPolygon(float directOcclusion, float reverbOcclusion, bool doubleSided, int numVertices, const FMOD_VECTOR *vertices, int *polygonIndex)
{
    ASSERT(vertices);
    ASSERT(numVertices >= 3);
    ASSERT(mNumPolygons < mMaxNumPolygons);
    ASSERT(mNumVertices + numVertices <= mMaxNumVertices);
    
    if (vertices == 0)
    {
         return FMOD_ERR_INVALID_PARAM;
    }
    if (numVertices < 3)
    {
         return FMOD_ERR_INVALID_PARAM;
    }
    if (mNumPolygons >= mMaxNumPolygons)
    {
         return FMOD_ERR_INVALID_PARAM;
    }
    if (mNumVertices + numVertices > mMaxNumVertices)
    {
         return FMOD_ERR_INVALID_PARAM;
    }

    if (polygonIndex)
    {
        *polygonIndex = mNumPolygons;
    }

    mNumVertices += numVertices;
    mPolygonOffsets[mNumPolygons] = mPolygonDataPos;
    FMOD_POLYGON& polygon = *(FMOD_POLYGON*)&mPolygonData[mPolygonDataPos];
    mPolygonDataPos += sizeof(FMOD_POLYGON) + (numVertices - 1) * sizeof (FMOD_VECTOR);
    mNumPolygons++;

    FMOD_memset(&polygon.node, 0, sizeof (OctreeNode));
    FMOD_memset(&polygon.nodeInternal, 0, sizeof (OctreeNode));
    polygon.directOcclusion = directOcclusion;
    polygon.reverbOcclusion = reverbOcclusion;
    polygon.flags = numVertices;
    if (doubleSided)
        polygon.flags |= FMOD_POLYGON_FLAG_DOUBLE_SIDED;

    int vertex;
    for (vertex = 0; vertex < (polygon.flags & FMOD_POLYGON_NUM_VERTICES_MASK); vertex++)
        (&polygon.vertices)[vertex] = vertices[vertex];    

    // add to update list
    polygon.node.nextItem = mPolygonUpdateList;
    mPolygonUpdateList = &polygon.node;

    setToBeUpdated();
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
FMOD_RESULT GeometryI::getNumPolygons(int *numPolygons)
{
    if (!numPolygons)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    *numPolygons = mNumPolygons;

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
FMOD_RESULT GeometryI::getMaxPolygons(int *maxNumPolygons, int *maxVertices)
{
    if (maxNumPolygons)
    {
        *maxNumPolygons = mMaxNumPolygons;
    }
    if (maxVertices)
    {
        *maxVertices = mMaxNumVertices;
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
FMOD_RESULT GeometryI::getPolygonNumVertices    (int polygonIndex, int *numVertices)
{
    ASSERT(polygonIndex >= 0);
    ASSERT(polygonIndex < mNumPolygons);
    ASSERT(mPolygonOffsets[polygonIndex] >= 0);
    ASSERT(mPolygonOffsets[polygonIndex] < mPolygonDataPos);
    
    if (polygonIndex < 0 || polygonIndex >= mNumPolygons)
    {
         return FMOD_ERR_INVALID_PARAM;
    }

    FMOD_POLYGON& polygon = *(FMOD_POLYGON*)&mPolygonData[mPolygonOffsets[polygonIndex]];

    if (numVertices)
    {
        *numVertices = (polygon.flags & FMOD_POLYGON_NUM_VERTICES_MASK);
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
FMOD_RESULT GeometryI::setPolygonVertex(int polygonIndex, int vertexIndex, const FMOD_VECTOR *vertex)
{    
    ASSERT(polygonIndex >= 0);
    ASSERT(polygonIndex < mNumPolygons);
    ASSERT(mPolygonOffsets[polygonIndex] >= 0);
    ASSERT(mPolygonOffsets[polygonIndex] < mPolygonDataPos);
    
    if (polygonIndex < 0 || polygonIndex >= mNumPolygons)
    {
         return FMOD_ERR_INVALID_PARAM;
    }
    
    FMOD_POLYGON& polygon = *(FMOD_POLYGON*)&mPolygonData[mPolygonOffsets[polygonIndex]];
    ASSERT(vertexIndex >= 0);
    ASSERT(vertexIndex < (polygon.flags & FMOD_POLYGON_NUM_VERTICES_MASK));
    ASSERT(vertex);

    if (vertexIndex < 0 || vertexIndex >= (polygon.flags & FMOD_POLYGON_NUM_VERTICES_MASK))
    {
         return FMOD_ERR_INVALID_PARAM;
    }

    if (vertex == 0)
    {
         return FMOD_ERR_INVALID_PARAM;
    }

    if ((&polygon.vertices)[vertexIndex].x == vertex->x &&
        (&polygon.vertices)[vertexIndex].y == vertex->y &&
        (&polygon.vertices)[vertexIndex].z == vertex->z)
    {
        return FMOD_OK;
    }

    (&polygon.vertices)[vertexIndex] = *vertex;

    // if we are in the octreee then remove and add to the update list
    if (polygon.node.flags & OCTREE_FLAG_INSERTED)
    {
        mOctree.deleteItem(&polygon.node);
        polygon.node.nextItem = mPolygonUpdateList;
        mPolygonUpdateList = &polygon.node;
    }

    setToBeUpdated();
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
FMOD_RESULT GeometryI::getPolygonVertex(int polygonIndex, int vertexIndex, FMOD_VECTOR *vertex)
{
    ASSERT(polygonIndex >= 0);
    ASSERT(polygonIndex < mNumPolygons);
    ASSERT(mPolygonOffsets[polygonIndex] >= 0);
    ASSERT(mPolygonOffsets[polygonIndex] < mPolygonDataPos);

    if (polygonIndex < 0 || polygonIndex >= mNumPolygons || !vertex)
    {
         return FMOD_ERR_INVALID_PARAM;
    }

    FMOD_POLYGON& polygon = *(FMOD_POLYGON*)&mPolygonData[mPolygonOffsets[polygonIndex]];
    ASSERT(vertexIndex >= 0);
    ASSERT(vertexIndex < (polygon.flags & FMOD_POLYGON_NUM_VERTICES_MASK));

    if (vertexIndex < 0 || vertexIndex >= (polygon.flags & FMOD_POLYGON_NUM_VERTICES_MASK))
    {
         return FMOD_ERR_INVALID_PARAM;
    }
   
    *vertex = (&polygon.vertices)[vertexIndex];

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
FMOD_RESULT GeometryI::setPolygonAttributes(int polygonIndex, float directOcclusion, float reverbOcclusion, bool doubleSided)
{
    ASSERT(polygonIndex >= 0);
    ASSERT(polygonIndex < mNumPolygons);
    ASSERT(mPolygonOffsets[polygonIndex] >= 0);
    ASSERT(mPolygonOffsets[polygonIndex] < mPolygonDataPos);
    if (polygonIndex < 0 || polygonIndex >= mNumPolygons)
    {
         return FMOD_ERR_INVALID_PARAM;
    }

    FMOD_POLYGON& polygon = *(FMOD_POLYGON*)&mPolygonData[mPolygonOffsets[polygonIndex]];
    polygon.directOcclusion = directOcclusion;
    polygon.reverbOcclusion = reverbOcclusion;
    if (doubleSided)
    {
        polygon.flags |= FMOD_POLYGON_FLAG_DOUBLE_SIDED;
    }
    else
    {
        polygon.flags &= ~FMOD_POLYGON_FLAG_DOUBLE_SIDED;
    }

    setToBeUpdated();
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
FMOD_RESULT GeometryI::getPolygonAttributes(int polygonIndex, float *directOcclusion, float *reverbOcclusion, bool *doubleSided)
{    
    ASSERT(polygonIndex >= 0);
    ASSERT(polygonIndex < mNumPolygons);
    ASSERT(mPolygonOffsets[polygonIndex] >= 0);
    ASSERT(mPolygonOffsets[polygonIndex] < mPolygonDataPos);

    if (polygonIndex < 0 || polygonIndex >= mNumPolygons)
    {
         return FMOD_ERR_INVALID_PARAM;
    }

    {
        FMOD_POLYGON &polygon = *(FMOD_POLYGON*)&mPolygonData[mPolygonOffsets[polygonIndex]];
    
        if (directOcclusion)
        {
            *directOcclusion = polygon.directOcclusion;
        }
        if (reverbOcclusion)
        {
            *reverbOcclusion = polygon.reverbOcclusion;
        }
        if (doubleSided)
        {
            *doubleSided = (polygon.flags & FMOD_POLYGON_FLAG_DOUBLE_SIDED) != 0;
        }
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
// VERIFY: bcbdad04 manual
// OFFSET: 0x0091A2C0
FMOD_RESULT GeometryI::flush()
{
    /*
    */
    int vertex;

    // iterate through polygons in the update list, calucate normals, aabb and add to octree.
    OctreeNode* node = mPolygonUpdateList;
    mPolygonUpdateList = 0;
    while (node)
    {
        OctreeNode* next = node->nextItem;
        node->nextItem = 0;

        FMOD_POLYGON& polygon = *(FMOD_POLYGON*)node;
        FMOD_VECTOR* vertices =& polygon.vertices;

        // calculate polygon normal
        float xN = 0.0f;
        float yN = 0.0f;
        float zN = 0.0f;
        // todo: return an error if a polygon has less then 3 vertices.
        for (vertex = 0; vertex < (polygon.flags & FMOD_POLYGON_NUM_VERTICES_MASK) - 2; vertex++)
        {
            float xA = vertices[vertex + 1].x -vertices[0].x;
            float yA = vertices[vertex + 1].y -vertices[0].y;
            float zA = vertices[vertex + 1].z -vertices[0].z;
            float xB = vertices[vertex + 2].x -vertices[0].x;
            float yB = vertices[vertex + 2].y -vertices[0].y;
            float zB = vertices[vertex + 2].z -vertices[0].z;
            // cross product
            xN += yA * zB - zA * yB;
            yN += zA * xB - xA * zB;
            zN += xA * yB - yA * xB;
        }    
        float fMagnidued = (float)sqrt(xN * xN + yN * yN + zN * zN);
        if (fMagnidued > 0.0f) // a tollerance here might be called for
        {
            polygon.flags &= ~FMOD_POLYGON_FLAG_INVALID;
            xN /= fMagnidued;
            yN /= fMagnidued;
            zN /= fMagnidued;
        }
        else
        {
            polygon.flags |= FMOD_POLYGON_FLAG_INVALID;
        }
        polygon.normal.x = xN;
        polygon.normal.y = yN;
        polygon.normal.z = zN;
        polygon.distance = FMOD_Vector_DotProduct(&vertices[0], &polygon.normal);
        
        // add to octree    
        mOctree.addInternalNode(&polygon.nodeInternal);
        FMOD_AABB& aabb = polygon.node.aabb;
        aabb.xMax = aabb.xMin = vertices[0].x;
        aabb.yMax = aabb.yMin = vertices[0].y;
        aabb.zMax = aabb.zMin = vertices[0].z;        
        for (vertex = 1; vertex < (polygon.flags & FMOD_POLYGON_NUM_VERTICES_MASK); vertex++)
        {
            aabb.xMax = MAX(aabb.xMax, vertices[vertex].x);
            aabb.xMin = MIN(aabb.xMin, vertices[vertex].x);
            aabb.yMax = MAX(aabb.yMax, vertices[vertex].y);
            aabb.yMin = MIN(aabb.yMin, vertices[vertex].y);
            aabb.zMax = MAX(aabb.zMax, vertices[vertex].z);
            aabb.zMin = MIN(aabb.zMin, vertices[vertex].z);
        }
        // we need to grow the bounding box a little to cope with the
        // case of an axis alligned polygon that might be right on the edge of a
        // bounding box.
        // Becaouse the line is clamped to each box in the tree before testing against
        // the polygon, we need to make the boxes a little bit bigger then the
        // polygons to make sure the line still passes through them.
        float fTollerance = aabb.xMax - aabb.xMin;
        fTollerance = MAX(fTollerance, aabb.yMax - aabb.yMin);
        fTollerance = MAX(fTollerance, aabb.zMax - aabb.zMin);
        fTollerance *= 0.01f;
        aabb.xMin -= fTollerance;
        aabb.xMax += fTollerance;
        aabb.yMin -= fTollerance;
        aabb.yMax += fTollerance;
        aabb.zMin -= fTollerance;
        aabb.zMax += fTollerance;
        
        if ((polygon.flags & FMOD_POLYGON_FLAG_INVALID) == 0)
        {
            mOctree.insertItem(&polygon.node);
        }
        node = next;
    }

    // update spacial data
    //mOctree.calculateAverageNodeElements(); // just for testing
    mOctree.getAABB(&mAABB);
    updateSpatialData();
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
FMOD_RESULT GeometryI::setActive(bool active)
{
    setToBeUpdated();

    mActive = active;

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
FMOD_RESULT GeometryI::getActive(bool *active)
{
    if (!active)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    *active = mActive;

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
FMOD_RESULT GeometryI::setRotation(const FMOD_VECTOR* forward, const FMOD_VECTOR* up)
{

    if (!forward)
    {
         return FMOD_ERR_INVALID_PARAM;
    }
    if (!up)
    {
         return FMOD_ERR_INVALID_PARAM;
    }

    if (mForward.x == forward->x && 
        mForward.y == forward->y && 
        mForward.z == forward->z && 
        mUp.x == up->x && 
        mUp.y == up->y && 
        mUp.z == up->z)
    {
        return FMOD_OK;
    }

    mForward = *forward;
    mUp = *up;
    calculateMatrix();
    setToBeUpdated();
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
FMOD_RESULT GeometryI::getRotation(FMOD_VECTOR* forward, FMOD_VECTOR* up)
{
    if (forward)
    {
        *forward = mForward;
    }

    if (up)
    {
        *up = mUp;
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
FMOD_RESULT GeometryI::setPosition(const FMOD_VECTOR* position)
{

    if (!position)
    {
         return FMOD_ERR_INVALID_PARAM;
    }

    if (mPosition.x == position->x && mPosition.y == position->y && mPosition.z == position->z)
    {
        return FMOD_OK;
    }
    
    mPosition = *position;

    setToBeUpdated();

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
FMOD_RESULT GeometryI::getPosition(FMOD_VECTOR* position)
{
    if (!position)
    {
        return FMOD_ERR_INVALID_PARAM;
    }
    
    *position = mPosition;

    return FMOD_OK;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Macintosh, XBox, PlayStation 2, GameCube

	[SEE_ALSO]
]
*/
FMOD_RESULT GeometryI::setScale(const FMOD_VECTOR* scale)
{

    if (!scale)
    {
         return FMOD_ERR_INVALID_PARAM;
    }

    if (scale->x == 0.0f || scale->y == 0.0f || scale->z == 0.0f)
    {
         return FMOD_ERR_INVALID_PARAM;
    }

    if (mScale.x == scale->x && 
        mScale.y == scale->y && 
        mScale.z == scale->z)
    {
        return FMOD_OK;
    }
    mScale = *scale;
    calculateMatrix();
    setToBeUpdated();
    return FMOD_OK;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Macintosh, XBox, PlayStation 2, GameCube

	[SEE_ALSO]
]
*/
FMOD_RESULT GeometryI::getScale(FMOD_VECTOR *scale)
{    
    if (!scale)
    {
        return FMOD_ERR_INVALID_PARAM;
    }
    
    *scale = mScale;

    return FMOD_OK;
}

/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Macintosh, XBox, PlayStation 2, GameCube

	[SEE_ALSO]
]
*/
FMOD_RESULT GeometryI::save(void *data, int *dataSize)
{    
    FMOD_RESULT result = FMOD_OK;

    if (!dataSize)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    if (data)
    {
        int tmpDataSize = *dataSize;
        result = serialiser(data, &tmpDataSize, true, false, saveData);
    }
    else
    {
        result = serialiser(data, dataSize, false, false, countData);
    }

    return result;
}

/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Macintosh, XBox, PlayStation 2, GameCube

	[SEE_ALSO]
]
*/
FMOD_RESULT GeometryI::load(const void *data, int dataSize)
{   

    if (!data)
    {
        return FMOD_ERR_INVALID_PARAM;
    }
    
    return serialiser((void*)data, &dataSize, false, true, loadData);
}

/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Macintosh, XBox, PlayStation 2, GameCube

	[SEE_ALSO]
]
*/
FMOD_RESULT GeometryI::saveData(void* fileData, int dataSize, int* fileDataIndex, void* liveData, int liveDataSize)
{
    if (*fileDataIndex + liveDataSize > dataSize)
        return FMOD_ERR_INVALID_PARAM;
    
#ifdef PLATFORM_ENDIAN_BIG
    if (liveDataSize == 4)
    {
        *(unsigned int*)&((unsigned char*)fileData)[*fileDataIndex] = *(unsigned int*)liveData;
    }
    else
    {
        FMOD_memcpy(&((unsigned char*)fileData)[*fileDataIndex], liveData, liveDataSize);
    }
#else
    FMOD_memcpy(&((unsigned char*)fileData)[*fileDataIndex], liveData, liveDataSize);
#endif

    *fileDataIndex += liveDataSize;

    return FMOD_OK;
}

/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Macintosh, XBox, PlayStation 2, GameCube

	[SEE_ALSO]
]
*/
FMOD_RESULT GeometryI::loadData(void* fileData, int dataSize, int* fileDataIndex, void* liveData, int liveDataSize)
{
    if (*fileDataIndex + liveDataSize > dataSize)
        return FMOD_ERR_INVALID_PARAM;

#ifdef PLATFORM_ENDIAN_BIG
    if (liveDataSize == 4)
    {
        *(unsigned int*)liveData = *(unsigned int*)&((unsigned char*)fileData)[*fileDataIndex];
    }
    else
    {
        FMOD_memcpy(liveData, &((unsigned char*)fileData)[*fileDataIndex], liveDataSize);
    }
#else
    FMOD_memcpy(liveData, &((unsigned char*)fileData)[*fileDataIndex], liveDataSize);
#endif

    *fileDataIndex += liveDataSize;
    return FMOD_OK;
}

/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Macintosh, XBox, PlayStation 2, GameCube

	[SEE_ALSO]
]
*/
FMOD_RESULT GeometryI::countData(void* fileData, int dataSize, int* fileDataIndex, void* liveData, int liveDataSize)
{
    *fileDataIndex += liveDataSize;
    return FMOD_OK;
}

/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Macintosh, XBox, PlayStation 2, GameCube

	[SEE_ALSO]
]
*/
FMOD_RESULT GeometryI::serialiser(
    void *data, 
    int *dataSize, 
    bool bWrite, 
    bool bRead, 
    FMOD_RESULT (*serialiseData)(void* fileData, int dataSize, int* fileDataIndex, void* liveData, int liveDataSize))
{ 
    bool bCount = !bWrite && !bRead;
   
    #define _CHECK_RESULT(x)                      \
    {                                             \
        FMOD_RESULT result = (x);                 \
        if (result != FMOD_OK)                    \
        {                                         \
            if (vertexArray)                      \
               FMOD_Memory_Free(vertexArray);     \
            return result;                        \
        }                                         \
    }

    FMOD_VECTOR* vertexArray = 0;
    int index = 0;

    unsigned int header;
    const unsigned int realHeader = 'F' + ('M' << 8) + ('O' << 16) + ('D' << 24);
    header = realHeader;
    _CHECK_RESULT(serialiseData(data, *dataSize, &index, &header, sizeof (int)))
    if (header != realHeader)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    int dataSizeCheck = *dataSize;
    _CHECK_RESULT(serialiseData(data, *dataSize, &index, &dataSizeCheck, sizeof (int)))
    if (bRead)
    {
        if (*dataSize != dataSizeCheck)
        {
            return FMOD_ERR_INVALID_PARAM;
        }
    }

    int numPolygons;
    if (bWrite || bCount)
    {
        _CHECK_RESULT(getNumPolygons(&numPolygons))
    }
    _CHECK_RESULT(serialiseData(data, *dataSize, &index, &numPolygons, sizeof (int)))

    int maxPolygons;
    int maxVertices;
    if (bWrite || bCount)
    {
        _CHECK_RESULT(getMaxPolygons(&maxPolygons, &maxVertices))
    }
    _CHECK_RESULT(serialiseData(data, *dataSize, &index, &maxPolygons, sizeof (int)))
    _CHECK_RESULT(serialiseData(data, *dataSize, &index, &maxVertices, sizeof (int)))

    int maxNumVerticesInOnPolygon = 64; // not very likely that this will be broken
    vertexArray = (FMOD_VECTOR*)FMOD_Memory_Alloc(maxNumVerticesInOnPolygon * sizeof (FMOD_VECTOR));
    if (!vertexArray)
        return FMOD_ERR_MEMORY;

    if (bRead)
    {
        _CHECK_RESULT(alloc(maxPolygons, maxVertices))
    }
    for (int polygonIndex = 0; polygonIndex < numPolygons; polygonIndex++)
    {
        int numVertices;
        if (bWrite || bCount)
        {
            _CHECK_RESULT(getPolygonNumVertices(polygonIndex, &numVertices))
        }
        _CHECK_RESULT(serialiseData(data, *dataSize, &index, &numVertices, sizeof (int)))

        if (maxNumVerticesInOnPolygon < numVertices)
        {
            // in the unlikly event we have more then 64 vertices in one polygon we need to 
            // reallocate the array
            FMOD_Memory_Free(vertexArray);
            vertexArray = (FMOD_VECTOR*)FMOD_Memory_Alloc(maxNumVerticesInOnPolygon * sizeof (FMOD_VECTOR));
            if (!vertexArray)
                return FMOD_ERR_MEMORY;
        }

        for (int vertexIndex = 0; vertexIndex < numVertices; vertexIndex++)
        {
           // FMOD_VECTOR vertex = FMOD_Memory_Alloc(mMaxNumVertices * sizeof (int));
            if (bWrite)
            {
                _CHECK_RESULT(getPolygonVertex(polygonIndex, vertexIndex, &vertexArray[vertexIndex]))
            }
            _CHECK_RESULT(serialiseData(data, *dataSize, &index, &vertexArray[vertexIndex].x, sizeof (float)))
            _CHECK_RESULT(serialiseData(data, *dataSize, &index, &vertexArray[vertexIndex].y, sizeof (float)))
            _CHECK_RESULT(serialiseData(data, *dataSize, &index, &vertexArray[vertexIndex].z, sizeof (float)))
        }

        float directOcclusion;
        float reverbOcclusion;
        bool bDoubleSided = false;;
        if (bWrite || bCount)
        {
            _CHECK_RESULT(getPolygonAttributes(polygonIndex, &directOcclusion, &reverbOcclusion, &bDoubleSided))
        }
        int doubleSided = bDoubleSided ? 1 : 0;
        _CHECK_RESULT(serialiseData(data, *dataSize, &index, &directOcclusion, sizeof (int)))
        _CHECK_RESULT(serialiseData(data, *dataSize, &index, &reverbOcclusion, sizeof (int)))
        _CHECK_RESULT(serialiseData(data, *dataSize, &index, &doubleSided, sizeof (int)))
        bDoubleSided = doubleSided != 0;

        if (bRead)
        {
            _CHECK_RESULT(addPolygon(directOcclusion, reverbOcclusion, bDoubleSided, numVertices, vertexArray, 0))
        }
    }

    FMOD_Memory_Free(vertexArray);
    vertexArray = 0;

    FMOD_VECTOR forward;
    FMOD_VECTOR up;
    if (bWrite || bCount)
    {
        _CHECK_RESULT(getRotation(&forward, &up))
    }
    _CHECK_RESULT(serialiseData(data, *dataSize, &index, &forward.x, sizeof (float)))
    _CHECK_RESULT(serialiseData(data, *dataSize, &index, &forward.y, sizeof (float)))
    _CHECK_RESULT(serialiseData(data, *dataSize, &index, &forward.z, sizeof (float)))
    _CHECK_RESULT(serialiseData(data, *dataSize, &index, &up.x, sizeof (float)))
    _CHECK_RESULT(serialiseData(data, *dataSize, &index, &up.y, sizeof (float)))
    _CHECK_RESULT(serialiseData(data, *dataSize, &index, &up.z, sizeof (float)))
    if (bRead)
    {
        _CHECK_RESULT(setRotation(&forward, &up))
    }

    FMOD_VECTOR position;
    if (bWrite || bCount)
    {
        _CHECK_RESULT(getPosition(&position))
    }
    _CHECK_RESULT(serialiseData(data, *dataSize, &index, &position.x, sizeof (float)))
    _CHECK_RESULT(serialiseData(data, *dataSize, &index, &position.y, sizeof (float)))
    _CHECK_RESULT(serialiseData(data, *dataSize, &index, &position.z, sizeof (float)))
    if (bRead)
    {
        _CHECK_RESULT(setPosition(&position))
    }

    FMOD_VECTOR scale;
    if (bWrite || bCount)
    {
        _CHECK_RESULT(getScale(&scale))
    }
    _CHECK_RESULT(serialiseData(data, *dataSize, &index, &scale.x, sizeof (float)))
    _CHECK_RESULT(serialiseData(data, *dataSize, &index, &scale.y, sizeof (float)))
    _CHECK_RESULT(serialiseData(data, *dataSize, &index, &scale.z, sizeof (float)))
    if (bRead)
    {
        _CHECK_RESULT(setScale(&scale))
    }
        
#undef _CHECK_RESULT

    if (bRead || bWrite)
    {
        if (*dataSize != index)
        {
            return FMOD_ERR_INVALID_PARAM;
        }
    }
    else
    {
        *dataSize = index;
    }


    return FMOD_OK;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Macintosh, XBox, PlayStation 2, GameCube

	[SEE_ALSO]
]
*/
FMOD_RESULT GeometryI::setUserData(void *userdata)
{
    mUserData = userdata;
    
    return FMOD_OK;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Macintosh, XBox, PlayStation 2, GameCube

	[SEE_ALSO]
]
*/
FMOD_RESULT GeometryI::getUserData(void **userdata)
{
    if (!userdata)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    *userdata = mUserData;
    
    return FMOD_OK;
}

/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Macintosh, XBox, PlayStation 2, GameCube

	[SEE_ALSO]
]
*/
// VERIFY: e9630d06 manual
// OFFSET: 0x0091A5A0
bool GeometryI::lineTest(LineTestData* lineTestData)
{
    FMOD_VECTOR startBck = lineTestData->start;
    FMOD_VECTOR endBck = lineTestData->end;

    FMOD_VECTOR startTmp = lineTestData->start;
    startTmp.x -= mPosition.x;
    startTmp.y -= mPosition.y;
    startTmp.z -= mPosition.z;
    FMOD_VECTOR endTmp = lineTestData->end;
    endTmp.x -= mPosition.x;
    endTmp.y -= mPosition.y;
    endTmp.z -= mPosition.z;

    matrixMult(mInvMatrix, &startTmp, &lineTestData->start);
    matrixMult(mInvMatrix, &endTmp, &lineTestData->end);

    bool bResult = mOctree.testLine(octreeLineTestCallback, lineTestData, lineTestData->start, lineTestData->end);

    lineTestData->start = startBck;
    lineTestData->end = endBck;

    lineTestData->geometryI = 0;
    return bResult;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Macintosh, XBox, PlayStation 2, GameCube

	[SEE_ALSO]
]
*/
FMOD_RESULT GeometryI::setWorldSize(float worldSize)
{
    mOctree.setMaxSize(worldSize);
    int poly;
    for (poly = 0; poly < mNumPolygons; poly++)
    {
        FMOD_POLYGON& polygon = *(FMOD_POLYGON*)&mPolygonData[mPolygonOffsets[poly]];
        mOctree.deleteItem(&polygon.node);
    }
    for (poly = 0; poly < mNumPolygons; poly++)
    {
        FMOD_POLYGON& polygon = *(FMOD_POLYGON*)&mPolygonData[mPolygonOffsets[poly]];
        polygon.node.nextItem = mPolygonUpdateList;
        mPolygonUpdateList = &polygon.node;
    }
    setToBeUpdated();

    return FMOD_OK;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Macintosh, XBox, PlayStation 2, GameCube

	[SEE_ALSO]
]
*/
void GeometryI::removeFromTree()
{
    mGeometryMgr->mainOctree()->deleteItem(&mSpatialData->octreeNode);
}


#if defined(FMOD_GEOMETRY_DEBUGGING)
/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Macintosh, XBox, PlayStation 2, GameCube

	[SEE_ALSO]
]
*/
void GeometryI::testLineTestForEachPolygon()
{
    for (int poly = 0; poly < mNumPolygons; poly++)
    {
        FMOD_POLYGON& polygon = *(FMOD_POLYGON*)&mPolygonData[mPolygonOffsets[poly]];
        if (polygon.flags & FMOD_POLYGON_FLAG_INVALID)
            continue;
        FMOD_VECTOR center = { 0.0f, 0.0f, 0.0f };
        int numVertices = (polygon.flags & FMOD_POLYGON_NUM_VERTICES_MASK);
        ASSERT(numVertices >= 3);
        for (int i = 0; i < numVertices; i++)
        {
            center.x += (&polygon.vertices)[i].x;
            center.y += (&polygon.vertices)[i].y;
            center.z += (&polygon.vertices)[i].z;
        }
        center.x /= numVertices;
        center.y /= numVertices;
        center.z /= numVertices;
        FMOD_VECTOR start;
        start.x = center.x - polygon.normal.x * 100.0f;
        start.y = center.y - polygon.normal.y * 100.0f;
        start.z = center.z - polygon.normal.z * 100.0f;
        FMOD_VECTOR end;
        end.x = center.x + polygon.normal.x * 100.0f;
        end.y = center.y + polygon.normal.y * 100.0f;
        end.z = center.z + polygon.normal.z * 100.0f;
        polygon.flags |= 0x80000000;
        float a, b;
        mGeometryMgr->lineTestAll(&start, &end, &a, &b);
        ASSERT((polygon.flags & 0x80000000) == 0);
        polygon.flags &= ~0x80000000;
    }
}
#endif

/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Macintosh, XBox, PlayStation 2, GameCube

	[SEE_ALSO]
]
*/
FMOD_RESULT GeometryI::validate(Geometry *geometry, GeometryI **geometryi)
{
    if (!geometryi)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    if (!geometry)
    {
        return FMOD_ERR_INVALID_HANDLE;
    }

    *geometryi = (GeometryI *)geometry;

    return FMOD_OK;
}



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
