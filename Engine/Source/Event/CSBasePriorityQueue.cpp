#include <cstdint>
#include "Engine/Source/Event/CSBasePriorityQueue.h"

// VERIFY: 95aa06ab auto
// OFFSET: 0x0047DE10
CSBasePriorityQueue::~CSBasePriorityQueue() {
    for (uint32_t i = 0; i < this->Count(); i++) {
        CSBasePriority* priority = this->Link(this->m_data[i]);
        priority->m_queue = nullptr;
        priority->m_index = 0;
    }
}

// VERIFY: 3c6be56a auto
// OFFSET: 0x0047E120
void CSBasePriorityQueue::Enqueue(void* ptr) {
    uint32_t index = this->Count();
    this->New();

    while (index) {
        uint32_t parent = (index - 1) >> 1;

        if (this->Link(this->m_data[parent])->Compare(this->Link(ptr))) {
            break;
        }

        this->m_data[index] = this->m_data[parent];

        CSBasePriority* priority = this->Link(this->m_data[index]);
        priority->m_index = index;
        priority->m_queue = this;

        index = parent;
    }

    this->m_data[index] = ptr;

    CSBasePriority* priority = this->Link(this->m_data[index]);
    priority->m_index = index;
    priority->m_queue = this;
}

// VERIFY: 10e10768 manual
// OFFSET: 0x0047E1D0
void CSBasePriorityQueue::Remove(uint32_t index) {
    CSBasePriority* removed = this->Link(this->m_data[index]);
    removed->m_queue = nullptr;
    removed->m_index = 0;

    void* last = *this->Top();
    uint32_t count = this->Count() - 1;
    this->SetCount(count);

    if (count == index) {
        return;
    }

    uint32_t lastIndex = count - 1;
    uint32_t lastParent = (count - 2) >> 1;

    if (index < lastIndex && index <= lastParent) {
        while (true) {
            uint32_t child = 2 * index + 1;

            if (child < lastIndex && this->Link(this->m_data[child + 1])->Compare(this->Link(this->m_data[child]))) {
                child++;
            }

            if (this->Link(last)->Compare(this->Link(this->m_data[child]))) {
                break;
            }

            this->m_data[index] = this->m_data[child];

            CSBasePriority* priority = this->Link(this->m_data[index]);
            priority->m_index = index;
            priority->m_queue = this;

            index = child;

            if (child > lastParent) {
                break;
            }
        }
    }

    this->m_data[index] = last;

    CSBasePriority* priority = this->Link(this->m_data[index]);
    priority->m_queue = this;
    priority->m_index = index;
}
