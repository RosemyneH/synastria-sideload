#include "fmod_settings.h"

#include "fmod_memory.h"
#include "fmod_metadata.h"
#include "fmod_string.h"

#include <string.h>


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
// VERIFY: cb779e96 manual
// OFFSET: 0x0091CD10
FMOD_RESULT TagNode::init(FMOD_TAGTYPE type, const char *name, void *data, unsigned int datalen, FMOD_TAGDATATYPE datatype)
{
    int memsize;

    mName = FMOD_strdup(name);
    if (!mName)
    {
        return FMOD_ERR_MEMORY;
    }

    memsize = datalen;
    if (datatype == FMOD_TAGDATATYPE_STRING)
    {
        memsize += 1;
    }

    //AJS double-buffered stuff!!!

    mData[0] = FMOD_Memory_Calloc(memsize);
    if (!mData[0])
    {
        return FMOD_ERR_MEMORY;
    }
    FMOD_memcpy(mData[0], data, datalen);

    mDataLen       = datalen;
    mType          = type;
    mDataType      = datatype;
    mUpdated       = true;
    mUnique        = false;
    mCurrentBuffer = 0;

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
// VERIFY: 6d564434 auto
// OFFSET: 0x0091CDB0
FMOD_RESULT TagNode::release()
{
    if (mName)
    {
        FMOD_Memory_Free(mName);
        mName = 0;
    }

    //AJS double-buffered!!!

    if (mData[0])
    {
        FMOD_Memory_Free(mData[0]);
        mData[0] = 0;
    }

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
// VERIFY: 2ef86b92 manual
// OFFSET: 0x0091CE20
FMOD_RESULT TagNode::update(void *data, unsigned int datalen)
{
    //AJS double-buffered stuff!!!!!!!!!!1

    if (mDataLen == datalen)
    {
        if (!memcmp(mData[0], data, datalen))
        {
            mUpdated = true;

            return FMOD_OK;
        }
    }

    if (mData[0])
    {
        FMOD_Memory_Free(mData[0]);
        mData[0] = 0;
    }

    mData[0] = FMOD_Memory_Alloc(datalen);
    if (!mData[0])
    {
        return FMOD_ERR_MEMORY;
    }
    FMOD_memcpy(mData[0], data, datalen);
    mDataLen = datalen;

    mUpdated = true;

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
// VERIFY: 78d6e61d auto
// OFFSET: 0x0091CF40
FMOD_RESULT Metadata::release()
{
    TagNode *node = (TagNode *)mList.getNext();

    while (node != &mList)
    {
        TagNode *next = (TagNode *)node->getNext();
        node->removeNode();
        node->release();
        node = next;
    }

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
// VERIFY: 82b299b7 auto
// OFFSET: 0x0091CFF0
FMOD_RESULT Metadata::getNumTags(int *numtags, int *numtagsupdated)
{
    TagNode *node;
    LinkedListNode *current;
    int num = 0, numupdated = 0;

    current = mList.getNext();
    while (current != &mList)
    {
        num++;
        node = (TagNode *)current;
        if (node->mUpdated)
        {
            numupdated++;
        }
        current = current->getNext();
    }

    if (numtags)
    {
        *numtags = num;
    }
    if (numtagsupdated)
    {
        *numtagsupdated = numupdated;
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
// VERIFY: 9758c992 manual
// OFFSET: 0x0091D030
FMOD_RESULT Metadata::getTag(const char *name, int index, FMOD_TAG *tag)
{
    int i;
    TagNode *node;
    LinkedListNode *current;

    /*
        index = 0-n, name = 0       Get the 0-n tag of the whole tag list
        index = 0-n, name = ""      Get the 0-n instance of the named tag
        index = -1,  name = 0       Get the first updated tag
        index = -1,  name = ""      Get the first updated instance of the named tag
    */

    if (index >= 0)
    {
        if (!name)
        {
            node = (TagNode *)mList.getNodeByIndex(index);
            if (!node)
            {
                return FMOD_ERR_TAGNOTFOUND;
            }
        }
        else
        {
            i = 0;
            current = mList.getNext();
            for (;;)
            {
                if (current == &mList)
                {
                    return FMOD_ERR_TAGNOTFOUND;
                }
                node = (TagNode *)current;
                if (!FMOD_strcmp(node->mName, name))
                {
                    if (i == index)
                    {
                        break;
                    }
                    i++;
                }
                current = current->getNext();
            }
        }
    }
    else
    {
        if (!name)
        {
            current = mList.getNext();
            for (;;)
            {
                if (current == &mList)
                {
                    return FMOD_ERR_TAGNOTFOUND;
                }
                node = (TagNode *)current;
                if (node->mUpdated)
                {
                    break;
                }
                current = current->getNext();
            }
        }
        else
        {
            current = mList.getNext();
            for (;;)
            {
                if (current == &mList)
                {
                    return FMOD_ERR_TAGNOTFOUND;
                }
                node = (TagNode *)current;
                if (node->mUpdated && !FMOD_strcmp(node->mName, name))
                {
                    break;
                }
                current = current->getNext();
            }
        }
    }

    tag->type     = node->mType;
    tag->datatype = node->mDataType;
    tag->name     = node->mName;
    tag->data     = node->mData[0];
    tag->datalen  = node->mDataLen;
    tag->updated  = node->mUpdated;

    if (node->mUpdated)
    {
        //AJS flip the buffer stuff...
        node->mUpdated = false;
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
// VERIFY: 43fdfdd9 auto
// OFFSET: 0x0091D140
FMOD_RESULT Metadata::add(Metadata *metadata)
{
    TagNode *node = (TagNode *)metadata->mList.getNext();

    while (node != &metadata->mList)
    {
        TagNode *next = (TagNode *)node->getNext();
        node->removeNode();

        if (node->mUnique)
        {
            LinkedListNode *current = mList.getNext();
            TagNode *existingnode = 0;

            for (;;)
            {
                if (current == &mList)
                {
                    existingnode = 0;
                    break;
                }
                existingnode = (TagNode *)current;
                if (!FMOD_strcmp(existingnode->mName, node->mName))
                {
                    break;
                }
                current = current->getNext();
            }

            if (existingnode)
            {
                //AJS double-buffered!!!!!1
                existingnode->update(node->mData[0], node->mDataLen);
                node->release();
            }
            else
            {
                addTag(node);
            }
        }
        else
        {
            addTag(node);
        }

        node = next;
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
FMOD_RESULT Metadata::addTag(TagNode *node)
{
    node->addBefore(&mList);

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
// VERIFY: f844a4d8 manual
// OFFSET: 0x0091D1E0
FMOD_RESULT Metadata::addTag(FMOD_TAGTYPE type, const char *name, void *data, unsigned int datalen, FMOD_TAGDATATYPE datatype, bool unique)
{
    FMOD_RESULT     result;
    TagNode        *node = 0;
    LinkedListNode *current;

    if (unique)
    {
        current = mList.getNext();
        for (;;)
        {
            if (current == &mList)
            {
                node = 0;
                break;
            }
            node = (TagNode *)current;
            if (!FMOD_strcmp(node->mName, name) && (node->mType == type))
            {
                break;
            }
            current = current->getNext();
        }
    }

    if (node && unique)
    {
        result = node->update(data, datalen);
    }
    else
    {
        node = FMOD_Object_Alloc(TagNode);
        if (!node)
        {
            return FMOD_ERR_MEMORY;
        }

        node->init(type, name, data, datalen, datatype);
        result = addTag(node);
    }

    if (unique)
    {
        node->mUnique = true;
    }

    return result;
}


}


