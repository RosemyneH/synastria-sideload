#include "fmod_octree.h"

#ifdef FMOD_SUPPORT_GEOMETRY

// Tree algorithim general description:
//
// The aim of this tree is for a more flexible and
// memory predictable octree. This is done by attemting
// to represent and octree where not all levels in the
// tree are explicitly stored. Buy only storing the
// necessary splits, we can garantee that the number
// of internal nodes will not be greater then the 
// number of leaf nodes.
//  
// The space is seperated by a series of seperating planes.
// Space is spit exactly in half along the z-y plane, then each
// subspace is split in half along the x-z plane, then each
// new subspace is exactlysplit in half along the x-y plane.
// Then this is repeated upto 31 times fitting into 32 bit
// precission. (Similar to a classical oct tree)
//
// Items have a bounding box with gives them a maximum extent.
// This limits the depth that they can be placed in to the
// tree.
//
// Splitting planes are only actually created and stored in
// memory if there are actually to groups of items that are
// seperated by the splitting plane.
//
// Each internal node is given an AABB that encloses its 
// children so that the tree can be easily traversed.
//
//
// Problems:
// If a lot of leaf nodes are in the same position they will
// end up in a list at the one node being very inefficient.


namespace FMOD
{

#if defined(_DEBUG) && defined(FMOD_GEOMETRY_DEBUGGING)
#define ASSERT(x) do { if (!(x)) __asm { int 3 } } while (false)
#else
#define ASSERT(x)
#endif

#if !(defined(_DEBUG) && defined(FMOD_GEOMETRY_DEBUGGING))
#define checkTree(x) {}
#define checkNodeIsContained(x) {}
#endif

// used to have a splitValue variable. This macro was a safe way to replace it.
#define SPLIT_VALUE(x) (x)->pos[(x)->flags & OCTREE_FLAG_SPLIT_MASK]
//#define SPLIT_VALUE(x) (x)->splitValue

    
//template <class Type> static inline SWAP(Type& x, Type& y) { Type tmp = x; x = y; y = tmp; }

template <class Type> 
class Swap
{
    Type tmp;

    Swap(Type& x, Type& y)
    {
        tmp = x;
        x = y;
        y = tmp;
    }
};

#define SWAP(x, y) { Swap swap(x, y); }


static inline float MAX(float x, float y) { return x > y ? x : y; }
static inline float MIN(float x, float y) { return x < y ? x : y; }

/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Macintosh, XBox, XBox360, PlayStation 2, GameCube

	[SEE_ALSO]
]
*/
static unsigned int HighestBit(unsigned int value)
{
    // clear all but the highest bit.
    // will a be nice acembler instructions to help with this on some machines.
    unsigned int v = value >> 1; 
    // propergate the highest bit down so all bits are filled
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    return value & ~v;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Macintosh, XBox, XBox360, PlayStation 2, GameCube

	[SEE_ALSO]
]
*/
// VERIFY: 383dec98 auto
// OFFSET: 0x00925520
void aabbAdd(FMOD_AABB& a, FMOD_AABB& b, FMOD_AABB& dst)
{    
    dst.xMin = MIN(a.xMin, b.xMin);
    dst.xMax = MAX(a.xMax, b.xMax);

    dst.yMin = MIN(a.yMin, b.yMin);
    dst.yMax = MAX(a.yMax, b.yMax);

    dst.zMin = MIN(a.zMin, b.zMin);
    dst.zMax = MAX(a.zMax, b.zMax);
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Macintosh, XBox, XBox360, PlayStation 2, GameCube

	[SEE_ALSO]
]
*/
Octree::Octree(float worldSize)
{
    ASSERT(worldSize != 0);

    mScale = 1.0f / worldSize;
    mCenter.x = 0.0f;
    mCenter.y = 0.0f;
    mCenter.z = 0.0f;

    mRoot = 0;
    mFreeList = 0;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Macintosh, XBox, XBox360, PlayStation 2, GameCube

	[SEE_ALSO]
]
*/
Octree::~Octree()
{
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Macintosh, XBox, XBox360, PlayStation 2, GameCube

	[SEE_ALSO]
]
*/
// VERIFY: 454f2183 auto
// OFFSET: 0x009262C0
void Octree::insertItem(OctreeNode* item)
{
//    checkTree(mRoot);
    // Insert a new item into the octree
    if (item->flags & OCTREE_FLAG_INSERTED)
        return;
    ASSERT(!item->parent);
    ASSERT(!item->nextItem);
    ASSERT(!item->hi);
    ASSERT(!item->lo);

    item->flags |= OCTREE_FLAG_LEAF | OCTREE_FLAG_INSERTED;
    FMOD_VECTOR extent;
    extent.x = item->aabb.xMax - item->aabb.xMin;
    extent.y = item->aabb.yMax - item->aabb.yMin;
    extent.z = item->aabb.zMax - item->aabb.zMin;
    float fMaxExtent;
    fMaxExtent = MAX(extent.x, extent.y);
    fMaxExtent = MAX(fMaxExtent, extent.z);
    fMaxExtent *= mScale  * (float)(1 << 30);
    unsigned int maxExtent = ftoint(fMaxExtent);
    maxExtent = HighestBit(maxExtent);
    item->splitLevel = maxExtent;
    item->pos[0] = xGetCenter(item);
    item->pos[1] = yGetCenter(item);
    item->pos[2] = zGetCenter(item);
    if (!mRoot)
    {
        mRoot = item;
        checkTree(mRoot);
        return;
    }

    insertInternal(mRoot, item);
//    checkTree(mRoot);
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Macintosh, XBox, XBox360, PlayStation 2, GameCube

	[SEE_ALSO]
]
*/
// VERIFY: dc64b5cd manual
// OFFSET: 0x009263F0
void Octree::deleteItem(OctreeNode* item)
{
    checkTree(mRoot);
    if ((item->flags & OCTREE_FLAG_INSERTED) == 0)
        return;

    ASSERT(item->flags & OCTREE_FLAG_LEAF);
    if (!item->parent)
    {
        // Item is the root
        ASSERT(mRoot == item);
        mRoot = item->nextItem;
        if (mRoot)
        {
            mRoot->parent = 0;
            mRoot->flags &= ~OCTREE_FLAG_EXTRA_NODE;
        }
        item->nextItem = 0;
        item->flags &= ~(OCTREE_FLAG_LEAF | OCTREE_FLAG_EXTRA_NODE | OCTREE_FLAG_INSERTED | OCTREE_FLAG_SPLIT_MASK | OCTREE_FLAG_CALCULATED_AABB);
    }
    else
    {       
        ASSERT(item);
        ASSERT(item->parent);
        OctreeNode* other = 0;
        if (item->parent->nextItem == item)
        {
            item->parent->nextItem = item->nextItem;
            if (item->nextItem)
            {
                item->nextItem->parent = item->parent;
            }
        }
        else
        if (item->parent->hi == item)
        {
            ASSERT((item->parent->flags & OCTREE_FLAG_LEAF) == 0);
            item->parent->hi = item->nextItem;
            if (item->nextItem)
            {
                item->nextItem->flags &= ~OCTREE_FLAG_EXTRA_NODE;
                item->nextItem->parent = item->parent;
            }
            else
            {
                ASSERT(item->parent->hi == 0);
                other = item->parent->lo;
                ASSERT(other);
            }
        }
        else
        {
            ASSERT((item->parent->flags & OCTREE_FLAG_LEAF) == 0);
            ASSERT(item->parent->lo == item);
            item->parent->lo = item->nextItem;
            if (item->nextItem)
            {
                item->nextItem->flags &= ~OCTREE_FLAG_EXTRA_NODE;
                item->nextItem->parent = item->parent;
            }
            else
            {
                ASSERT(item->parent->lo == 0);
                other = item->parent->hi;
                ASSERT(other);
            }
        }
        if (other)
        {
            // the items parent only has one child
            // the parent needs to be deleted
            ASSERT(!item->nextItem);
            ASSERT(item->parent);
            OctreeNode* parent = item->parent;
            OctreeNode* nextItem = parent->nextItem;
            OctreeNode* reinsertAt;
            ASSERT((parent->flags & OCTREE_FLAG_LEAF) == 0);
            if (parent->parent == 0)
            {
                mRoot = other;
                other->parent = 0;
                reinsertAt = mRoot;
            }
            else
            {
                ASSERT((parent->parent->flags & OCTREE_FLAG_LEAF) == 0);
                ASSERT(parent->parent->hi == parent || parent->parent->lo == parent);
                if (parent->parent->hi == parent)
                    parent->parent->hi = other;
                else
                    parent->parent->lo = other;
                other->parent = parent->parent;
                reinsertAt = parent->parent;
            }
            // items stored at the deleted parent need to be delt with.
            // we will reinsert them
            // actually, we might be able to just add then to the grand parent
            // which would be faster but we will do this to make sure
            while (nextItem)
            {
                OctreeNode* next = nextItem->nextItem;
                nextItem->nextItem = 0;
                nextItem->parent = 0;
                nextItem->flags &= ~OCTREE_FLAG_EXTRA_NODE;
                insertInternal(reinsertAt, nextItem);
                nextItem = next;
            }
            if (parent->parent)
            {
                ASSERT(parent->parent->lo->parent == parent->parent);
                ASSERT(parent->parent->hi->parent == parent->parent);
                adjustAABBs(parent->parent);
            }

            parent->parent = 0;
            parent->hi = 0;
            parent->lo = 0;
            parent->nextItem = 0;
            parent->flags &= ~(OCTREE_FLAG_SPLIT_MASK | OCTREE_FLAG_CALCULATED_AABB);
            addToFreeList(parent);
            checkTree(mRoot);
        }
        else
        {            
            if ((item->parent->flags & OCTREE_FLAG_LEAF) == 0)
            {
                ASSERT(item->parent->lo->parent == item->parent);            
                ASSERT(item->parent->hi->parent == item->parent);
                adjustAABBs(item->parent);
            }
        }
        item->parent = 0;
        item->nextItem = 0;
        item->flags &= ~(OCTREE_FLAG_LEAF | OCTREE_FLAG_EXTRA_NODE | OCTREE_FLAG_INSERTED | OCTREE_FLAG_SPLIT_MASK | OCTREE_FLAG_CALCULATED_AABB);
    }
    checkTree(mRoot);
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Macintosh, XBox, XBox360, PlayStation 2, GameCube

	[SEE_ALSO]
]
*/
// VERIFY: ab66def8 manual
// OFFSET: 0x00926580
void Octree::updateItem(OctreeNode* item)
{
    // this function must be called if an items aabb has been changed.
    // if the aabb has changed enough, the item must be deleted and
    // inserted, otherwise the aabbs are just ajusted up the tree.
    if (item->flags & OCTREE_FLAG_INSERTED)
    {
        // check if the node acctually needs to change place in the tree.
        FMOD_VECTOR extent;
        extent.x = item->aabb.xMax - item->aabb.xMin;
        extent.y = item->aabb.yMax - item->aabb.yMin;
        extent.z = item->aabb.zMax - item->aabb.zMin;
        float fMaxExtent;
        fMaxExtent = MAX(extent.x, extent.y);
        fMaxExtent = MAX(fMaxExtent, extent.z);
        fMaxExtent *= mScale  * (float)(1 << 30);
        unsigned int maxExtent = ftoint(fMaxExtent);
        maxExtent = HighestBit(maxExtent);
        unsigned int pos[3];
        pos[0] = xGetCenter(item);
        pos[1] = yGetCenter(item);
        pos[2] = zGetCenter(item);
        unsigned int mask = ~(item->splitLevel + 0xffffffff);
        // might give better results to check against the parent
        // here but we will try to keep things simple
        if (maxExtent == item->splitLevel && 
            (pos[0] & mask) == (item->pos[0] & mask) &&
            (pos[1] & mask) == (item->pos[1] & mask) &&
            (pos[2] & mask) == (item->pos[2] & mask))
        {
            // The item hasn't changed enough to need to be reinserted.
            // We still might need to addjust the aabbs
            adjustAABBs(item);
            return;
        }
        // we need to delete the item and reinsert
        deleteItem(item);
    }
    insertItem(item);
}

// VERIFY: a03a183c auto
// OFFSET: 0x009255C0
void Octree::getAABB(FMOD_AABB* aabb)
{
    if (mRoot)
    {
        *aabb = mRoot->aabb;
        // this will be very slow in the pathological case.
        for (OctreeNode* next = mRoot->nextItem; next; next = next->nextItem)
            aabbAdd(next->aabb, *aabb, *aabb);
    }
    else
    {
        aabb->xMin = 0.0f;
        aabb->xMax = 0.0f;
        aabb->yMin = 0.0f;
        aabb->yMax = 0.0f;
        aabb->zMin = 0.0f;
        aabb->zMax = 0.0f;
    }
}

// VERIFY: 6d0a5201 auto
// OFFSET: 0x00925E20
void Octree::addInternalNode(OctreeNode* item)
{
    ASSERT(item);
    if (item->flags & OCTREE_FLAG_INTERNAL_NODE)
        return;

    item->flags |= OCTREE_FLAG_INTERNAL_NODE;
    addToFreeList(item);
}

void Octree::removeInternalNode(OctreeNode* item)
{
    checkTree(mRoot);
    ASSERT(item);
    if ((item->flags & OCTREE_FLAG_INTERNAL_NODE) == 0)
        return;
    item->flags &= ~OCTREE_FLAG_INTERNAL_NODE;
    ASSERT((item->flags & OCTREE_FLAG_LEAF) == 0);
    // we need to free the memory used by this node
    if (item->flags & OCTREE_FLAG_FREE)
    {
        // just remove from the free list
        if (item->parent)
        {
            item->parent->nextItem = item->nextItem;
            if (item->nextItem)
            {
                item->nextItem->parent = item->parent;
            }
        }
        else
        {
            ASSERT(mFreeList == item);
            mFreeList = item->nextItem;
            if (mFreeList)
                mFreeList->parent = 0;
        }
        checkTree(mRoot);
    }
    else
    {
        // replace with a new node
        OctreeNode* replacement = getFreeNode();

        *replacement = *item;
        replacement->flags &= ~OCTREE_FLAG_INTERNAL_NODE;
        replacement->flags |= OCTREE_FLAG_INTERNAL_NODE;
        if (replacement->parent)
        {
            if (replacement->parent->nextItem == item)
            {
                replacement->parent->nextItem = replacement;
            }
            else
            if (replacement->parent->hi == item)
            {
                replacement->parent->hi = replacement;
            }
            else
            {
                ASSERT(replacement->parent->lo == item);
                replacement->parent->lo = replacement;
            }
        }
        else
        {
            mRoot = replacement;
        }
        if (replacement->nextItem)
            replacement->nextItem->parent = replacement;
        if (replacement->hi)
            replacement->hi->parent = replacement;
        if (replacement->lo)
            replacement->lo->parent = replacement;
        checkTree(mRoot);
    }
}

#if defined(_DEBUG) && defined(FMOD_GEOMETRY_DEBUGGING)
void Octree::checkTree(OctreeNode* node)
{
    if (!node)
        return;
    ASSERT((node->flags & OCTREE_FLAG_FREE) == 0);
    if (node->parent)
    {
        if ((node->flags & OCTREE_FLAG_LEAF) == 0)
        {
            ASSERT(node->parent->splitLevel >= node->splitLevel);

            if (node->parent->splitLevel == node->splitLevel)
            {
                ASSERT((node->parent->flags & OCTREE_FLAG_SPLIT_MASK) < (node->flags & OCTREE_FLAG_SPLIT_MASK));
            }
        }
        if ((node->parent->flags & OCTREE_FLAG_LEAF) == 0)
        {
            ASSERT((node->flags & OCTREE_FLAG_EXTRA_NODE) == 0);
        }
        if ((node->flags & OCTREE_FLAG_EXTRA_NODE) == 0)
        {
            ASSERT((node->parent->flags & OCTREE_FLAG_LEAF) == 0);
        }
        ASSERT(
            node->parent->lo == node ||
            node->parent->hi == node ||
            node->parent->nextItem == node);
        ASSERT((node->parent->flags & OCTREE_FLAG_FREE) == 0);
    }
    if (node->flags & OCTREE_FLAG_LEAF)
    {
        ASSERT(node->lo == 0);
        ASSERT(node->hi == 0);
        checkNodeIsContained(node);
        for (OctreeNode* next = node->nextItem; next; next = next->nextItem)
        {
            ASSERT(next->flags & OCTREE_FLAG_EXTRA_NODE);
            ASSERT(next->flags & OCTREE_FLAG_LEAF);
            ASSERT(next->splitLevel >= next->parent->splitLevel);
            if (node->parent)
            {
                ASSERT(
                    node->parent->lo == node ||
                    node->parent->hi == node ||
                    node->parent->nextItem == node);
            }
            checkNodeIsContained(next);
        }
    }
    else
    {
        ASSERT(node->lo);
        ASSERT(node->lo->parent == node);
        checkTree(node->lo);

        ASSERT(node->hi);
        ASSERT(node->lo->parent == node);
        checkTree(node->hi);
        checkNodeIsContained(node);
    }
    
}

void Octree::checkNodeIsContained(OctreeNode* node)
{
    //return; // skip this because it is often too slow even for debug

    if (!node)
        return;

    unsigned int xCenter = node->pos[0];
    unsigned int yCenter = node->pos[1];
    unsigned int zCenter = node->pos[2];

    OctreeNode* bottom = node;
    OctreeNode* first = 0;

    while (node->parent)
    {
        if ((node->parent->flags & OCTREE_FLAG_LEAF) == 0 &&
            node->parent->nextItem != node)
        {
            if (first == 0)
                first = node->parent;
            ASSERT(node->parent->lo == node || node->parent->hi == node);

            switch (node->parent->flags & OCTREE_FLAG_SPLIT_MASK)
            {
            case OCTREE_FLAG_X_SPLIT:
                {
                    if (node->parent->hi == node)
                    {
                        ASSERT(xCenter >= SPLIT_VALUE(node->parent));
                    }
                    else
                    {
                        ASSERT(xCenter < SPLIT_VALUE(node->parent));
                    }
                    if (bottom->splitLevel <= node->parent->splitLevel)
                    {
                        ASSERT(((bottom->pos[0] ^ node->parent->pos[0]) & ~((node->parent->splitLevel << 1) + 0xffffffff)) == 0);
                        ASSERT(((bottom->pos[1] ^ node->parent->pos[1]) & ~((node->parent->splitLevel << 1) + 0xffffffff)) == 0);
                        ASSERT(((bottom->pos[2] ^ node->parent->pos[2]) & ~((node->parent->splitLevel << 1) + 0xffffffff)) == 0);
                    }
                    else
                    {
                        ASSERT(first == node->parent);
                    }
                }
                break;
            case OCTREE_FLAG_Y_SPLIT:
                {
                    if (node->parent->hi == node)
                    {
                        ASSERT(yCenter >= SPLIT_VALUE(node->parent));
                    }
                    else
                    {
                        ASSERT(yCenter < SPLIT_VALUE(node->parent));
                    }
                    if (bottom->splitLevel <= node->parent->splitLevel)
                    {
                        ASSERT(((bottom->pos[0] ^ node->parent->pos[0]) & ~((node->parent->splitLevel << 0) + 0xffffffff)) == 0);
                        ASSERT(((bottom->pos[1] ^ node->parent->pos[1]) & ~((node->parent->splitLevel << 1) + 0xffffffff)) == 0);
                        ASSERT(((bottom->pos[2] ^ node->parent->pos[2]) & ~((node->parent->splitLevel << 1) + 0xffffffff)) == 0);
                    }
                    else
                    {
                        ASSERT(first == node->parent);
                    }
                }
                break;

            case OCTREE_FLAG_Z_SPLIT:
                {
                    if (node->parent->hi == node)
                    {
                        ASSERT(zCenter >= SPLIT_VALUE(node->parent));
                    }
                    else
                    {
                        ASSERT(zCenter < SPLIT_VALUE(node->parent));
                    }
                    if (bottom->splitLevel <= node->parent->splitLevel)
                    {
                        ASSERT(((bottom->pos[0] ^ node->parent->pos[0]) & ~((node->parent->splitLevel << 0) + 0xffffffff)) == 0);
                        ASSERT(((bottom->pos[1] ^ node->parent->pos[1]) & ~((node->parent->splitLevel << 0) + 0xffffffff)) == 0);
                        ASSERT(((bottom->pos[2] ^ node->parent->pos[2]) & ~((node->parent->splitLevel << 1) + 0xffffffff)) == 0);
                    }
                    else
                    {
                        ASSERT(first == node->parent);
                    }
                }
                break;
            }



        }
        node = node->parent;
    }
}

static int nonEmptyNodeCount = 0;
static int totalNodeSize = 0;
static int maxNodeSize = 0;
void Octree::calculateAverageNodeElements()
{
    nonEmptyNodeCount = 0;
    totalNodeSize = 0;
    maxNodeSize = 0;
    calculateAverageNodeElements(mRoot);

    int total = totalNodeSize;
    int count = nonEmptyNodeCount;
    int max = maxNodeSize;

    int test = 0;
}

void Octree::calculateAverageNodeElements(OctreeNode* node)
{
    OctreeNode* next; 
    if (node->flags & OCTREE_FLAG_LEAF)
        next = node;
    else
        next = node->nextItem;

    if (next)
    {
        nonEmptyNodeCount++;
        int count = 0;
        for (; next; next = next->nextItem)
        {
            totalNodeSize++;            
            count++;
        }
        if (maxNodeSize < count)
            maxNodeSize = count;
    }

    if (node->hi)
        calculateAverageNodeElements(node->hi);
    if (node->lo)
        calculateAverageNodeElements(node->lo);

}
#endif // _DEBUG

void Octree::setMaxSize(float maxSize)
{
    ASSERT(maxSize != 0);
    mScale = 1.0f / maxSize;
}

// VERIFY: 1c250df2 auto
// OFFSET: 0x00925E60
bool Octree::testLine(bool (*octreeLineTestCallback)(OctreeNode* item, void* data), void* data, const FMOD_VECTOR& a, const FMOD_VECTOR& b)
{
    ASSERT(octreeLineTestCallback);    
    if (mRoot)
    {
        RecursionData recursionData;
        recursionData.octreeLineTestCallback = octreeLineTestCallback;
        recursionData.data = data;
        recursionData.exit = false;
        testLine(mRoot, a, b, &recursionData);
        return (!recursionData.exit);
    }
    return true;
}

#if defined(FMOD_GEOMETRY_DEBUGGING)
void Octree::renderTree(void (*renderBox)(float xMin, float xMax, float yMin, float yMax, float zMin, float zMax))
{
    renderTreeInternal(mRoot, renderBox);
}

void Octree::renderTreeInternal(OctreeNode* node, void (*renderBox)(float xMin, float xMax, float yMin, float yMax, float zMin, float zMax))
{
    if (!node)
        return;
    renderBox(node->aabb.xMin, node->aabb.xMax, node->aabb.yMin, node->aabb.yMax, node->aabb.zMin, node->aabb.zMax);
    
    for (OctreeNode* next = node->nextItem; next; next = next->nextItem)
    {
        renderBox(next->aabb.xMin, next->aabb.xMax, next->aabb.yMin, next->aabb.yMax, next->aabb.zMin, next->aabb.zMax);
    }

    renderTreeInternal(node->hi, renderBox);
    renderTreeInternal(node->lo, renderBox);
}
#endif


#if defined(FMOD_23TREE)
void Octree::insert23Tree(OctreeNode* parent, OctreeNode** pParent, OctreeNode* item)
{
    item->flags |= OCTREE_FLAG_23LEAF;
    if (*pParent == 0)
    {
        // add item as root
        *pParent = item;
        item->parent = parent;
        item->flags |= OCTREE_FLAG_23ROOT;
    }
    else
    if ((*pParent)->flags & OCTREE_FLAG_23LEAF)
    {
        OctreeNode* other = *pParent;
        // make new root
        ASSERT(other->parent == parent);
        ASSERT(other->flags & OCTREE_FLAG_23ROOT);
        other->flags &= ~OCTREE_FLAG_23ROOT;
        OctreeNode* internalNode = getFreeNode();
        internalNode->flags |= OCTREE_FLAG_23NODE | OCTREE_FLAG_23ROOT;

        *pParent = internalNode;
        internalNode->parent = parent;

        if (other->splitLevel < item->splitLevel)
        {
            internalNode->lo = other;
            internalNode->hi = item;
        }
        else
        {
            internalNode->lo = other;
            internalNode->hi = item;
        }
        internalNode->pos[0] = internalNode->hi->splitLevel;
        internalNode->pos[2] = internalNode->lo->splitLevel; // store smallest in subtree here
        ASSERT(internalNode->nextItem == 0);
        internalNode->lo->parent = internalNode;
        internalNode->hi->parent = internalNode;
    }
    else
    {
        // find where to do the insertion
        OctreeNode* node = *pParent;
        while (node)
        {
            ASSERT(node->lo);
            ASSERT(node->hi);
            if (node->lo->flags & OCTREE_FLAG_23LEAF)
            {
                if (node->nextItem == 0) 
                {
                    ASSERT(node->pos[0] == node->hi->splitLevel);
                    // we have room for a new item at this leaf
                    if (item->splitLevel < node->lo->splitLevel)
                    {
                        node->pos[1] = node->pos[0];
                        node->nextItem = node->hi;
                        node->hi = node->lo;
                        node->pos[0] = node->hi->splitLevel;
                        node->lo = item;
                    }
                    else
                    if (item->splitLevel < node->hi->splitLevel)
                    {
                        node->nextItem = node->hi;
                        node->hi = item;
                        node->pos[0] = node->hi->splitLevel;
                        node->pos[1] = node->nextItem->splitLevel;
                    }
                    else
                    {
                        node->pos[1] = item->splitLevel;
                        node->nextItem = item;
                    }
                    item->parent = node;
                    ASSERT(node->pos[0] == node->hi->splitLevel);
                    ASSERT(node->pos[1] == node->nextItem->splitLevel);
                    ASSERT(node->lo->splitLevel <= node->hi->splitLevel);    
                    ASSERT(node->hi->splitLevel <= node->nextItem->splitLevel);    
                    ASSERT(node->pos[1] >= node->pos[0]);    
                    break;
                }
                else
                {
                    split23Tree(node, item);
                    break;
                }
            }
            else
            {
                if (item->splitLevel < node->pos[0])
                {
                    node = node->lo;
                }
                else 
                if (!node->nextItem)
                {
                    node = node->hi;
                }
                else 
                if (item->splitLevel < node->pos[1])
                {
                    node = node->hi;
                }
                else
                {
                    node = node->nextItem;
                }
            }
        }
    }
}

void Octree::split23Tree(OctreeNode* node, OctreeNode* item)
{    
    ASSERT(node->lo);
    ASSERT(node->hi);
    ASSERT(node->nextItem);
    ASSERT(node->lo->flags & OCTREE_FLAG_23LEAF);
    ASSERT(node->hi->flags & OCTREE_FLAG_23LEAF);
    ASSERT(node->nextItem->flags & OCTREE_FLAG_23LEAF);
    ASSERT(item->flags & OCTREE_FLAG_23LEAF);
    if (node->flags & OCTREE_FLAG_23ROOT)
    {

        if (item->splitLevel < node->nextItem->splitLevel)
            SWAP(item, node->nextItem);
        if (node->nextItem->splitLevel < node->hi->splitLevel)
            SWAP(node->nextItem, node->hi);
        if (node->hi->splitLevel < node->lo->splitLevel)
            SWAP(node->hi, node->lo);

        OctreeNode* internalNodeA = getFreeNode();
        internalNodeA->flags |= OCTREE_FLAG_23NODE;
        internalNodeA->lo = node->lo;
        internalNodeA->hi = node->hi;
        internalNodeA->lo->parent = internalNodeA;
        internalNodeA->hi->parent = internalNodeA;
        internalNodeA->pos[0] = internalNodeA->hi->splitLevel;

        OctreeNode* internalNodeB = getFreeNode();
        internalNodeB->flags |= OCTREE_FLAG_23NODE;
        internalNodeB->lo = node->nextItem;
        internalNodeB->hi = item;
        internalNodeB->lo->parent = internalNodeB;
        internalNodeB->hi->parent = internalNodeB;
        internalNodeB->pos[0] = internalNodeB->hi->splitLevel;

        node->nextItem = 0;
        node->lo = internalNodeA;
        node->hi = internalNodeB;
        node->lo->parent = node;
        node->hi->parent = node;
        node->pos[0] = node->hi->lo->splitLevel;
        return;
    }

    if (item->splitLevel < node->nextItem->splitLevel)
        SWAP(item, node->nextItem);
    if (node->nextItem->splitLevel < node->hi->splitLevel)
        SWAP(node->nextItem, node->hi);
    if (node->hi->splitLevel < node->lo->splitLevel)
        SWAP(node->hi, node->lo);

    OctreeNode* internalNode = getFreeNode();
    internalNode->flags |= OCTREE_FLAG_23NODE;
    internalNode->lo = node->nextItem;
    internalNode->hi = item;
    internalNode->lo->parent = internalNode;
    internalNode->hi->parent = internalNode;
    internalNode->pos[0] = internalNode->hi->splitLevel;
    unsigned int newSplit = internalNode->lo->splitLevel;

    ASSERT(node->lo->splitLevel <= node->hi->splitLevel);
    ASSERT(node->hi->splitLevel <= internalNode->lo->splitLevel);
    ASSERT(internalNode->lo->splitLevel <= internalNode->hi->splitLevel);

    node->nextItem = 0;
    node->lo->parent = node;
    node->hi->parent = node;
    node->pos[0] = node->hi->splitLevel;
    item = internalNode;

    for (;;)
    {
        ASSERT(node->flags & OCTREE_FLAG_23NODE);
        if (node->flags & OCTREE_FLAG_23ROOT)
        {
            OctreeNode* internalNode = getFreeNode();
            *internalNode = *node;
            ASSERT(internalNode->flags & OCTREE_FLAG_23NODE);
            internalNode->flags &= ~OCTREE_FLAG_23ROOT;
            internalNode->lo->parent = internalNode;
            internalNode->hi->parent = internalNode;
            node->pos[0] = newSplit;
            node->lo = internalNode;
            node->hi = item;
            node->lo->parent = node;
            node->hi->parent = node;
            break;
        }
        else
        {
            // try to insert the new internal node at the parent.
            ASSERT(node->parent);
            ASSERT(node->parent->flags & OCTREE_FLAG_23NODE);
            OctreeNode* parent = node->parent;
            if (parent->lo == node)
            {    
                if (parent->nextItem)
                {
                    // we need to split the parent
                    OctreeNode* internalNode = getFreeNode();
                    internalNode->flags |= OCTREE_FLAG_23NODE;
                    internalNode->lo = parent->hi;
                    internalNode->hi = parent->nextItem;
                    internalNode->pos[0] = parent->pos[1];
                    internalNode->lo->parent = internalNode;
                    internalNode->hi->parent = internalNode;

                    parent->hi = item;
                    parent->hi->parent = parent;
                    SWAP(parent->pos[0], newSplit);
                    //parent->pos[0] = newSplit;
                    parent->nextItem = 0;

                    item = internalNode;
                    node = parent;
                }
                else
                {
                    // add item here
                    parent->nextItem = parent->hi;
                    parent->pos[1] = parent->pos[0];
                    parent->pos[0] = newSplit;
                    parent->hi = item; 
                    parent->hi->parent = parent;
                    break;
                }
            }
            else
            if (parent->hi == node)
            {
                if (parent->nextItem)
                {
                    // we need to split the parent
                    OctreeNode* internalNode = getFreeNode();
                    internalNode->flags |= OCTREE_FLAG_23NODE;
                    internalNode->lo = item;
                    internalNode->hi = parent->nextItem;
                    internalNode->pos[0] = parent->pos[1];
                    internalNode->lo->parent = internalNode;
                    internalNode->hi->parent = internalNode;

                    parent->nextItem = 0;

                    item = internalNode;
                    node = parent;
                }
                else
                {
                    // add item here
                    parent->nextItem = item;
                    parent->nextItem->parent = parent;
                    parent->pos[1] = newSplit;
                    break;
                }
            }
            else
            {
                ASSERT(parent->nextItem == node);
                // we need to split the parent
                OctreeNode* internalNode = getFreeNode();
                internalNode->flags |= OCTREE_FLAG_23NODE;
                internalNode->lo = parent->nextItem;
                internalNode->hi = item;
                internalNode->pos[0] = newSplit;
                internalNode->lo->parent = internalNode;
                internalNode->hi->parent = internalNode;
                newSplit = parent->pos[1];

                parent->nextItem = 0;

                item = internalNode;
                node = parent;
            }
        }
    }
}

void Octree::remove23Tree(OctreeNode* item)
{
}

static unsigned int checkTreeValue = 0;
void Octree::check23Tree(OctreeNode* parent, OctreeNode** pParent)
{
    checkTreeValue = 0;
    check23TreeRecursive(*pParent);
}

void Octree::check23TreeRecursive(OctreeNode* node)
{
    if (!node)
        return;
    if (node->flags & OCTREE_FLAG_23LEAF)
    {
        ASSERT(node->splitLevel >= checkTreeValue);
        checkTreeValue = node->splitLevel;
        return;
    }
    ASSERT(node->flags & OCTREE_FLAG_23NODE);
    ASSERT(node->lo);
    ASSERT(node->hi);

    ASSERT(node->lo->parent == node);
    check23TreeRecursive(node->lo);

    ASSERT(node->hi->parent == node);
    check23TreeRecursive(node->hi);
    ASSERT(findLowest(node->hi) == node->pos[0]);
    if (node->nextItem)
    {
        ASSERT(node->nextItem->parent == node);
        check23TreeRecursive(node->nextItem);
        ASSERT(findLowest(node->nextItem) == node->pos[1]);
    }
}

int unsigned Octree::findLowest(OctreeNode* node)
{
    for (; (node->flags & OCTREE_FLAG_23LEAF) == 0; node = node->lo)
        ;
    return node->splitLevel;
}

/*
// testing the 2-3-tree insert
#include <memory.h>
#include <stdlib.h>
class TestClass
{
public:
    TestClass()
    {
        srand(0);
        const int numNodes = 5000;
        Octree octree;
        OctreeNode octreeNodes[numNodes];
        OctreeNode octreeInternalNodes[numNodes];
        FMOD_memset(octreeNodes, 0, sizeof (octreeNodes));
        FMOD_memset(octreeInternalNodes, 0, sizeof (octreeInternalNodes));
        int node;
        for (node = 0; node < numNodes; node++)
        {
            octree.addInternalNode(&octreeInternalNodes[node]);
        }

        OctreeNode* root = 0;
        for (node = 0; node < numNodes; node++)
        {
            octreeNodes[node].splitLevel = FMOD_RAND();
            octree.insert23Tree(0, &root, &octreeNodes[node]);
        //    octree.check23Tree(0, &root);
        }
        octree.check23Tree(0, &root);
    }
};
TestClass testClass;*/
#endif

// VERIFY: ef2c3bcf manual
// OFFSET: 0x00925EE0
void Octree::insertInternal(OctreeNode* node, OctreeNode* item)
{
    for (;;)
    {
        {
            // find the highest split with node.
            int splitAxis = 0;
            unsigned int highestSplit = 0;
            if (node->flags & OCTREE_FLAG_LEAF)
            {
                for (int i = 0; i < 3; i++)
                {
                    unsigned int tmp = (node->pos[i] ^ item->pos[i]);
                    tmp = HighestBit(tmp);
                    if (tmp > highestSplit  &&
                        tmp > item->splitLevel &&
                        tmp > node->splitLevel)
                    {
                        highestSplit = tmp;
                        splitAxis = i;
                    }
                }
            }
            else
            {
                for (int i = 0; i < 3; i++)
                {
                    unsigned int tmp = (node->pos[i] ^ item->pos[i]) & ~(node->splitLevel + 0xffffffff);
                    tmp = HighestBit(tmp);
                    if (highestSplit < tmp &&
                        tmp > item->splitLevel &&
                        (tmp > node->splitLevel || (tmp == node->splitLevel && i < (node->flags & OCTREE_FLAG_SPLIT_MASK))))
                    {
                        highestSplit = tmp;
                        splitAxis = i;
                    }
                }
            }


            if (highestSplit)
            {
                OctreeNode* newNode = getFreeNode();

                checkTree(mRoot);

                newNode->flags |= (newNode->flags & ~OCTREE_FLAG_SPLIT_MASK) | splitAxis;
            //    newNode->splitValue = item->pos[splitAxis] & (~(0xffffffff + highestSplit)) | highestSplit;
                newNode->splitLevel = highestSplit;

                
                ASSERT((item->pos[splitAxis] & newNode->splitLevel) != (node->pos[splitAxis]  & newNode->splitLevel));
                if (item->pos[splitAxis] & highestSplit)
                {
                    newNode->lo = node;
                    newNode->hi = item;
                }
                else
                {
                    newNode->hi = node;
                    newNode->lo = item;
                }

                newNode->parent = node->parent;
                newNode->hi->parent = newNode;
                newNode->lo->parent = newNode;
                ASSERT(newNode->splitLevel >= newNode->hi->splitLevel);
                ASSERT(newNode->splitLevel >= newNode->lo->splitLevel);
                if (newNode->parent)
                {
                    ASSERT(newNode->parent->splitLevel >= newNode->splitLevel);
                    ASSERT(newNode->parent->hi == node || newNode->parent->lo == node);
                    if (newNode->parent->lo == node)
                        newNode->parent->lo = newNode;
                    else
                        newNode->parent->hi = newNode;
                }
                else
                {
                    mRoot = newNode;
                }

                if (splitAxis == 0)
                {
                    newNode->pos[0] = item->pos[0] & (~(0xffffffff + (newNode->splitLevel >> 0))) | newNode->splitLevel;
                    newNode->pos[1] = item->pos[1] & (~(0xffffffff + (newNode->splitLevel >> 0))) | newNode->splitLevel;
                    newNode->pos[2] = item->pos[2] & (~(0xffffffff + (newNode->splitLevel >> 0))) | newNode->splitLevel;

                    ASSERT(((newNode->lo->pos[0] ^ newNode->pos[0]) & ~((newNode->splitLevel << 1) + 0xffffffff)) == 0);
                    ASSERT(((newNode->lo->pos[1] ^ newNode->pos[1]) & ~((newNode->splitLevel << 1) + 0xffffffff)) == 0);
                    ASSERT(((newNode->lo->pos[2] ^ newNode->pos[2]) & ~((newNode->splitLevel << 1) + 0xffffffff)) == 0);

                    ASSERT(((newNode->hi->pos[0] ^ newNode->pos[0]) & ~((newNode->splitLevel << 1) + 0xffffffff)) == 0);
                    ASSERT(((newNode->hi->pos[1] ^ newNode->pos[1]) & ~((newNode->splitLevel << 1) + 0xffffffff)) == 0);
                    ASSERT(((newNode->hi->pos[2] ^ newNode->pos[2]) & ~((newNode->splitLevel << 1) + 0xffffffff)) == 0);
                }
                else
                if (splitAxis == 1)
                {
                    newNode->pos[0] = item->pos[0] & (~(0xffffffff + (newNode->splitLevel >> 1))) | (newNode->splitLevel >> 1);
                    newNode->pos[1] = item->pos[1] & (~(0xffffffff + (newNode->splitLevel >> 0))) | newNode->splitLevel;
                    newNode->pos[2] = item->pos[2] & (~(0xffffffff + (newNode->splitLevel >> 0))) | newNode->splitLevel;

                    ASSERT(((newNode->lo->pos[0] ^ newNode->pos[0]) & ~((newNode->splitLevel << 0) + 0xffffffff)) == 0);
                    ASSERT(((newNode->lo->pos[1] ^ newNode->pos[1]) & ~((newNode->splitLevel << 1) + 0xffffffff)) == 0);
                    ASSERT(((newNode->lo->pos[2] ^ newNode->pos[2]) & ~((newNode->splitLevel << 1) + 0xffffffff)) == 0);

                    ASSERT(((newNode->hi->pos[0] ^ newNode->pos[0]) & ~((newNode->splitLevel << 0) + 0xffffffff)) == 0);
                    ASSERT(((newNode->hi->pos[1] ^ newNode->pos[1]) & ~((newNode->splitLevel << 1) + 0xffffffff)) == 0);
                    ASSERT(((newNode->hi->pos[2] ^ newNode->pos[2]) & ~((newNode->splitLevel << 1) + 0xffffffff)) == 0);

                }
                else
                {
                    newNode->pos[0] = item->pos[0] & (~(0xffffffff + (newNode->splitLevel >> 1))) | (newNode->splitLevel >> 1);
                    newNode->pos[1] = item->pos[1] & (~(0xffffffff + (newNode->splitLevel >> 1))) | (newNode->splitLevel >> 1);
                    newNode->pos[2] = item->pos[2] & (~(0xffffffff + (newNode->splitLevel >> 0))) | newNode->splitLevel;

                    ASSERT(((newNode->lo->pos[0] ^ newNode->pos[0]) & ~((newNode->splitLevel << 0) + 0xffffffff)) == 0);
                    ASSERT(((newNode->lo->pos[1] ^ newNode->pos[1]) & ~((newNode->splitLevel << 0) + 0xffffffff)) == 0);
                    ASSERT(((newNode->lo->pos[2] ^ newNode->pos[2]) & ~((newNode->splitLevel << 1) + 0xffffffff)) == 0);

                    ASSERT(((newNode->hi->pos[0] ^ newNode->pos[0]) & ~((newNode->splitLevel << 0) + 0xffffffff)) == 0);
                    ASSERT(((newNode->hi->pos[1] ^ newNode->pos[1]) & ~((newNode->splitLevel << 0) + 0xffffffff)) == 0);
                    ASSERT(((newNode->hi->pos[2] ^ newNode->pos[2]) & ~((newNode->splitLevel << 1) + 0xffffffff)) == 0);
                }
                ASSERT(newNode->hi->pos[splitAxis] >= SPLIT_VALUE(newNode));
                ASSERT(newNode->lo->pos[splitAxis] < SPLIT_VALUE(newNode));
                OctreeNode* next = node->nextItem;
                node->nextItem = 0;
                adjustAABBs(newNode);
                while (next)
                {
                    OctreeNode* nextNext = next->nextItem;
                    next->parent = 0;
                    next->nextItem = 0;
                    next->flags &= ~OCTREE_FLAG_EXTRA_NODE;
                    if (newNode->parent)
                        insertInternal(newNode->parent, next);
                    else
                        insertInternal(mRoot, next);
                    //insertInternal(mRoot, next);
                    next = nextNext;
                }



                checkNodeIsContained(newNode->hi);
                if (newNode->hi->nextItem)
                    checkNodeIsContained(newNode->hi->nextItem);
                checkNodeIsContained(newNode->lo);
                if (newNode->lo->nextItem)
                    checkNodeIsContained(newNode->lo->nextItem);
                checkTree(newNode);
                checkTree(mRoot);
                break;
            }
            else
            if (item->splitLevel >= node->splitLevel)
            {
                // the item is too big to go past this node so just add here.
                addListItem(node, item);
                adjustAABBs(node);

                // error checking
                checkNodeIsContained(item);
                checkNodeIsContained(node);
                checkTree(node);
                checkTree(mRoot);
                break;
            }
            else
            if (node->flags & OCTREE_FLAG_LEAF)
            {
                // the item is too big to go past this node so just add here.
                OctreeNode* parent = node->parent;
                addListItem(node, item);
                adjustAABBs(parent);

                // error checking
                checkNodeIsContained(item);
                checkNodeIsContained(node);
                checkTree(parent);
                checkTree(mRoot);
                break;
            }
            else
            {

                switch (item->pos[node->flags & OCTREE_FLAG_SPLIT_MASK])
                {
                case 0:
                    ASSERT(((item->pos[0] ^ node->pos[0]) & ~((node->splitLevel >> 1) + 0xffffffff)) == 0);
                    ASSERT(((item->pos[1] ^ node->pos[1]) & ~((node->splitLevel >> 1) + 0xffffffff)) == 0);
                    ASSERT(((item->pos[2] ^ node->pos[2]) & ~((node->splitLevel >> 1) + 0xffffffff)) == 0);
                    break;
                case 1:
                    ASSERT(((item->pos[0] ^ node->pos[0]) & ~((node->splitLevel >> 0) + 0xffffffff)) == 0);
                    ASSERT(((item->pos[1] ^ node->pos[1]) & ~((node->splitLevel >> 1) + 0xffffffff)) == 0);
                    ASSERT(((item->pos[2] ^ node->pos[2]) & ~((node->splitLevel >> 1) + 0xffffffff)) == 0);
                    break;
                case 2:
                    ASSERT(((item->pos[0] ^ node->pos[0]) & ~((node->splitLevel >> 0) + 0xffffffff)) == 0);
                    ASSERT(((item->pos[1] ^ node->pos[1]) & ~((node->splitLevel >> 0) + 0xffffffff)) == 0);
                    ASSERT(((item->pos[2] ^ node->pos[2]) & ~((node->splitLevel >> 1) + 0xffffffff)) == 0);
                    break;
                }

            //    ASSERT((item->pos[0] ^ node->pos[0]) < (node->splitLevel << 0));
            //    ASSERT((item->pos[1] ^ node->pos[1]) < (node->splitLevel << 0));
            //    ASSERT((item->pos[2] ^ node->pos[2]) < (node->splitLevel << 0));
                node = (item->pos[node->flags & OCTREE_FLAG_SPLIT_MASK] >= SPLIT_VALUE(node)) ? node->hi : node->lo;
                ASSERT((node->parent->flags & OCTREE_FLAG_LEAF) == 0);
            }
        }
    }
}

void Octree::addToFreeList(OctreeNode* item)
{
    item->nextItem = mFreeList;
    mFreeList = item;  
    if (item->nextItem)
        item->nextItem->parent = item;
    item->parent = 0;
    ASSERT((item->flags & OCTREE_FLAG_FREE) == 0);
    item->flags |= OCTREE_FLAG_FREE;
}

OctreeNode* Octree::getFreeNode()
{
    ASSERT(mFreeList);
    OctreeNode* node = mFreeList;
    mFreeList = mFreeList->nextItem;
    if (mFreeList)
        mFreeList->parent = 0;
    node->nextItem = 0;
    ASSERT(node->flags & OCTREE_FLAG_FREE);
    node->flags &= ~OCTREE_FLAG_FREE;
    return node;
}

// VERIFY: 5aa51e6e auto
// OFFSET: 0x00925640
void Octree::adjustAABBs(OctreeNode* node)
{
    while (node)
    {
        if ((node->flags & OCTREE_FLAG_LEAF) == 0)
        {
            ASSERT(node->hi);
            ASSERT(node->lo);
            aabbAdd(node->hi->aabb, node->lo->aabb, node->aabb);

            // todo: Adding all the items on this node to the aabb can be slow.
            // this could be made faster using a 2-3-tree for this list.
            // An easier way to speed this up would be to only add the new aabb
            // being added to the tree rather then recalculating everything at
            // each node as we go up the tree.
            for (OctreeNode* next = node->nextItem; next; next = next->nextItem)
            {
                aabbAdd(next->aabb, node->aabb, node->aabb);
            }
            node->flags |= OCTREE_FLAG_CALCULATED_AABB;
            // if a child is a leaf, we have to add any attacted items to the aabb of the parent.
            if (node->hi->flags & OCTREE_FLAG_LEAF)
            {
                for (OctreeNode* next = node->hi->nextItem; next; next = next->nextItem)
                {
                    aabbAdd(next->aabb, node->aabb, node->aabb);
                }
            }
            if (node->lo->flags & OCTREE_FLAG_LEAF)
            {
                for (OctreeNode* next = node->lo->nextItem; next; next = next->nextItem)
                {
                    aabbAdd(next->aabb, node->aabb, node->aabb);
                }
            }
        }
        node = node->parent;
    }
}


void Octree::removeListItem(OctreeNode* node)
{
    ASSERT(node);
    ASSERT(node->parent);
    if (node->parent->nextItem == node)
    {
        node->parent->nextItem = node->nextItem;
    }
    else
    if (node->parent->hi == node)
    {
        node->parent->hi = node->nextItem;
        if (node->nextItem)
            node->nextItem->flags &= ~OCTREE_FLAG_EXTRA_NODE;
    }
    else
    {
        ASSERT(node->parent->lo == node);
        node->parent->lo = node->nextItem;
        if (node->nextItem)
            node->nextItem->flags &= ~OCTREE_FLAG_EXTRA_NODE;
    }
    if (node->nextItem)
        node->nextItem->parent = node->parent;
    node->parent = 0;
    node->nextItem = 0;
    node->flags &= ~OCTREE_FLAG_EXTRA_NODE;
}

// VERIFY: a0b8a458 auto
// OFFSET: 0x009256E0
void Octree::addListItem(OctreeNode* list, OctreeNode* node)
{
    // add to list, sorted by extent
    ASSERT(list);
    ASSERT(node->nextItem == 0);
    ASSERT(node->parent == 0);
    if ((list->flags & OCTREE_FLAG_LEAF) == 0)
    {
        // don't insert before the first item if it isn't a leaf node
        if (list->nextItem)
        {
            list = list->nextItem;
        }
        else
        {
            // add after
            ASSERT(node->nextItem == 0);
            list->nextItem = node;
            node->parent = list;
            node->flags |= OCTREE_FLAG_EXTRA_NODE;
            return;
        }
    }

    while (node->splitLevel > list->splitLevel && list->nextItem)
        list = list->nextItem;

    if (!list->nextItem && node->splitLevel > list->splitLevel)
    {
        // add after
        ASSERT(node->nextItem == 0);
        list->nextItem = node;
        node->parent = list;
        node->flags |= OCTREE_FLAG_EXTRA_NODE;
    }
    else
    {
        // add before
        if (!list->parent)
        {
            ASSERT(mRoot == list);
            mRoot = node;
        }
        else
        if (list->parent->nextItem == list)
        {
            list->parent->nextItem = node;
            if (list->parent->flags & list->flags & OCTREE_FLAG_LEAF)
                node->flags |= OCTREE_FLAG_EXTRA_NODE;
        }
        else
        if (list->parent->hi == list)
        {
            list->parent->hi = node;
        }
        else
        {
            ASSERT(list->parent->lo == list);
            list->parent->lo = node;
        }
        node->parent = list->parent;
        node->nextItem = list;
        list->parent = node;
        list->flags |= OCTREE_FLAG_EXTRA_NODE;
    }   
    ASSERT(list->nextItem != list);
    ASSERT(node->nextItem != node);
}

// VERIFY: 3513f551 manual
// OFFSET: 0x00925790
void Octree::testLine(OctreeNode* node, FMOD_VECTOR a, FMOD_VECTOR b, RecursionData* recursionData)
{
    float t;
    float da;
    float db;
    
    for (OctreeNode* next = node->nextItem; next; next = next->nextItem)
    {
        ASSERT(next->flags & OCTREE_FLAG_LEAF);
        // todo: probably best to test this bounding box also.
        if (!recursionData->octreeLineTestCallback(next, recursionData->data))
        {
            recursionData->exit = true;
            return;
        }
    }    
    // clip line to box 
    // if the line does not intersect the box then return.

#define CLAMP_LINE()                                \
    if (da < 0.0f && db > 0.0f)                     \
    {                                               \
        t = da / (da - db);                         \
        a.x = a.x + t * (b.x - a.x);                \
        a.y = a.y + t * (b.y - a.y);                \
        a.z = a.z + t * (b.z - a.z);                \
    }                                               \
    else                                            \
    if (da > 0.0f && db < 0.0f)                     \
    {                                               \
        t = db / (db - da);                         \
        b.x = b.x + t * (a.x - b.x);                \
        b.y = b.y + t * (a.y - b.y);                \
        b.z = b.z + t * (a.z - b.z);                \
    }

#define CLAMP_MIN() CLAMP_LINE() else if (da < 0.0f && db < 0.0f) return;
#define CLAMP_MAX() CLAMP_LINE() else if (da < 0.0f && db < 0.0f) return;


    da = a.x - node->aabb.xMin;
    db = b.x - node->aabb.xMin;
    CLAMP_MIN()
    da = node->aabb.xMax - a.x;
    db = node->aabb.xMax - b.x;
    CLAMP_MAX()

    da = a.y - node->aabb.yMin;
    db = b.y - node->aabb.yMin;
    CLAMP_MIN()
    da = node->aabb.yMax - a.y;
    db = node->aabb.yMax - b.y;
    CLAMP_MAX()

    da = a.z - node->aabb.zMin;
    db = b.z - node->aabb.zMin;
    CLAMP_MIN()
    da = node->aabb.zMax - a.z;
    db = node->aabb.zMax - b.z;
    CLAMP_MAX()

    if (node->flags & OCTREE_FLAG_LEAF)
    {
        if (!recursionData->octreeLineTestCallback(node, recursionData->data))
        {
            recursionData->exit = true;
            return;
        }
    }
    else
    {
        if (node->hi)
        {
            testLine(node->hi, a, b, recursionData);
            if (recursionData->exit)
                return;
        }
        if (node->lo)
        {
            testLine(node->lo, a, b, recursionData);
            if (recursionData->exit)
                return;
        }
    }
}

}

#endif // FMOD_SUPPORT_GEOMETRY

