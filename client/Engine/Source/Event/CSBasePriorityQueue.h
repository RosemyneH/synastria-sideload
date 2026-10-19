#ifndef ENGINE_SOURCE_EVENT_CSBASEPRIORITYQUEUE_H
#define ENGINE_SOURCE_EVENT_CSBASEPRIORITYQUEUE_H

#include <cstddef>
#include <cstdint>
#include "Engine/Source/Storm/H/stpl.h"

class CSBasePriority;

class CSBasePriorityQueue : public TSGrowableArray<void*> {
    public:
    /* 0010 */ uint32_t m_linkOffset;

    ~CSBasePriorityQueue();
    void Enqueue(void* ptr);
    void Remove(uint32_t index);

    private:
    CSBasePriority* Link(void* ptr);
};

class CSBasePriority {
    public:
    /* 0004 */ CSBasePriorityQueue* m_queue;
    /* 0008 */ uint32_t m_index;

    CSBasePriority();
    ~CSBasePriority();
    virtual int32_t Compare(CSBasePriority* a) const = 0;
};

template <class T>
class TSPriorityQueue : public CSBasePriorityQueue {
    public:
    TSPriorityQueue(uint32_t linkOffset);
    T* Dequeue();
    T* Head();
};

template <class T>
class TSTimerPriority : public CSBasePriority {
    public:
    /* 000C */ T m_val;

    TSTimerPriority();
    int32_t Compare(CSBasePriority* a) const;
    void SetVal(const T& val);
};

// INLINED
inline CSBasePriority* CSBasePriorityQueue::Link(void* ptr) {
    return reinterpret_cast<CSBasePriority*>(static_cast<uint8_t*>(ptr) + this->m_linkOffset);
}

// INLINED
inline CSBasePriority::CSBasePriority() {
    this->m_queue = nullptr;
    this->m_index = 0;
}

// INLINED
inline CSBasePriority::~CSBasePriority() {
    if (this->m_queue) {
        this->m_queue->Remove(this->m_index);
    }
}

// INLINED
template <class T>
TSPriorityQueue<T>::TSPriorityQueue(uint32_t linkOffset) {
    this->m_linkOffset = linkOffset;
}

// INLINED
template <class T>
T* TSPriorityQueue<T>::Dequeue() {
    if (!this->Count()) {
        return nullptr;
    }

    T* ptr = static_cast<T*>(this->m_data[0]);
    this->Remove(0);

    return ptr;
}

// INLINED
template <class T>
T* TSPriorityQueue<T>::Head() {
    return this->Count() ? static_cast<T*>(this->m_data[0]) : nullptr;
}

// INLINED
template <class T>
TSTimerPriority<T>::TSTimerPriority() : m_val(0) {
}

// VERIFY: f1c04974 auto
// OFFSET: 0x0047E580
template <class T>
int32_t TSTimerPriority<T>::Compare(CSBasePriority* a) const {
    return static_cast<int32_t>(this->m_val - static_cast<TSTimerPriority<T>*>(a)->m_val) <= 0;
}

// INLINED
template <class T>
void TSTimerPriority<T>::SetVal(const T& val) {
    if (val == this->m_val) {
        return;
    }

    CSBasePriorityQueue* queue = this->m_queue;
    this->m_val = val;

    if (queue) {
        void* ptr = queue->m_data[this->m_index];
        queue->Remove(this->m_index);
        queue->Enqueue(ptr);
    }
}

#endif
