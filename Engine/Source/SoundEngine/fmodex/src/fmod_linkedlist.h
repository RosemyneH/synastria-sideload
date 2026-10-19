#ifndef _FMOD_LINKEDLIST_H
#define	_FMOD_LINKEDLIST_H

#include "fmod_types.h"

namespace FMOD
{
    class LinkedListNode
    {
        friend class SortedLinkedListNode;

      private:

	    LinkedListNode *mNodeNext;
	    LinkedListNode *mNodePrev;
        void           *mNodeData;

      public:
        LinkedListNode()  { initNode(); }
#ifdef FMOD_SUPPORT_RTTI
        virtual ~LinkedListNode()  { }
#endif

        inline LinkedListNode *getNext() const
        {
            return mNodeNext;
        }
        inline LinkedListNode *getPrev() const
        {
            return mNodePrev;
        }
        inline void setData(void *data)
        {
            mNodeData = data;
        }
        inline void *getData() const
        {
            return mNodeData;
        }
        inline void initNode()
        { 
            mNodePrev = mNodeNext = this; 
            mNodeData = 0;
        }
// VERIFY: 58be76e9 auto
        // OFFSET: 0x008D2680
        inline void removeNode()     
        { 
            mNodePrev->mNodeNext = mNodeNext; 
            mNodeNext->mNodePrev = mNodePrev; 
            mNodeNext     = mNodePrev = this; 
            mNodeData     = 0;
        }
        void addAfter(LinkedListNode *node)
        {
            mNodeNext            = node->mNodeNext;
            mNodePrev            = node;
            mNodeNext->mNodePrev = this;
            mNodePrev->mNodeNext = this;       
        }
        void addBefore(LinkedListNode *node)
        {
            mNodePrev            = node->mNodePrev;
            mNodeNext            = node;
            mNodeNext->mNodePrev = this;
            mNodePrev->mNodeNext = this;       
        }
        bool exists(LinkedListNode *node)
        {
            LinkedListNode *current = this->mNodeNext;

            do
            {
                if (current == node)
                {
                    return true;
                }
                current = current->mNodeNext;
            } while (current != this);

            return false;
        }
        // OFFSET: 0x008D0510
        inline bool isEmpty() const
        {
            if (mNodeNext == this && mNodePrev == this)
            {
                return true;
            }

            return false;
        }
        int count() const
        {
            int i = 0;
            LinkedListNode *current = this->mNodeNext;

            while (current != this)
            {
                i++;
                current = current->mNodeNext;
            }

            return i;
        }
        LinkedListNode *getNodeByIndex(int i) const
        {
            if (i < 0)
            {
                return 0;
            }

            LinkedListNode *current = this->mNodeNext;

            if (current == this)
            {
                return 0;
            }

            for (;i > 0;i--)
            {
                current = current->mNodeNext;

                if (current == this)
                {
                    return 0;
                }
            }

            return current;
        }
        // OFFSET: 0x008F3820
        int getNodeIndex(LinkedListNode *node) const
        {
            LinkedListNode *current = this->mNodeNext;

            for (int i=0; current != this; i++, current = current->mNodeNext)
            {
                if (current == node)
                {
                    return i;
                }
            }

            return -1;
        }
    };

    class SortedLinkedListNode : public LinkedListNode
    {
      private:
      
        unsigned int    mNodePriority;
      
      public:
        SortedLinkedListNode()  { initNode(); }
              
        inline void initNode()
        { 
            mNodePriority = (unsigned int)-1;
            mNodePrev     = mNodeNext = this; 
            mNodeData     = 0;
        }
        inline void removeNode()     
        { 
            mNodePrev->mNodeNext = mNodeNext; 
            mNodeNext->mNodePrev = mNodePrev; 
            mNodeNext            = mNodePrev = this; 
            mNodePriority        = (unsigned int)-1;
            mNodeData            = 0;
        }

        // OFFSET: 0x008DCD20
        void addAt(SortedLinkedListNode *head, unsigned int priority)
        {
            SortedLinkedListNode *current = SAFE_CAST(SortedLinkedListNode, head->mNodeNext);
            
            do
            {
                if (priority <= current->mNodePriority)
                {
                    mNodePriority = priority;
                    addBefore(current);
                    return;
                }
                current = SAFE_CAST(SortedLinkedListNode, current->mNodeNext);
            } while (current->mNodePrev != head);
        }
    };

}




#endif




