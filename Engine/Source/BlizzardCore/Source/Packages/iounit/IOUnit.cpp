#include <cstdint>
#include <new>
#include "Engine/Source/BlizzardCore/Source/Packages/iounit/IOUnit.h"
#include "Engine/Source/BlizzardCore/Source/Packages/iounit/IOUnitContainer.h"

// INLINED
IOUnit::IOUnit(IOUnit* next, bool ownsMemory) {
    this->m_next = next;
    this->m_ownsMemory = ownsMemory;
    this->m_refCount.m_value = 1;
}

// VERIFY: 16519fc9 auto
// OFFSET: 0x00426550
bool IOUnit::Read(void* buffer, int64_t offset, uint32_t count, uint32_t* bytesRead) {
    return this->m_next->Read(buffer, offset, count, bytesRead);
}

// VERIFY: f8faffdf manual
// OFFSET: 0x00426580
bool IOUnit::Write(const void* buffer, int64_t offset, uint32_t count) {
    return this->m_next->Write(buffer, offset, count);
}

// VERIFY: ec8568a0 auto
// OFFSET: 0x004265B0
bool IOUnit::Flush() {
    return this->m_next->Flush();
}

// VERIFY: 73c65392 auto
// OFFSET: 0x004265C0
bool IOUnit::GetFileSize(int64_t* size) {
    return this->m_next->GetFileSize(size);
}

// VERIFY: 438935f2 auto
// OFFSET: 0x004265D0
bool IOUnit::SetFileSize(int64_t size) {
    return this->m_next->SetFileSize(size);
}

// VERIFY: 8ecd0d4d manual
// OFFSET: 0x004265F0
void* IOUnit::Win32Handle() {
    return this->m_next->Win32Handle();
}

// VERIFY: eb064776 auto
// OFFSET: 0x00430BE0
IOUnit::~IOUnit() {
}

bool IOUnit::Close() {
    return true;
}

// VERIFY: 230d82c9 manual
// OFFSET: 0x00430C10
bool IOUnit::Release() {
    bool result = true;
    IOUnit* unit = this;

    do {
        if (unit->m_refCount.Decrement() > 0) {
            break;
        }

        if (!unit->Close()) {
            result = false;
        }

        unit = unit->m_next;
    } while (unit);

    IOUnit* stop = unit;
    unit = this;

    while (unit != stop) {
        IOUnit* next = unit->m_next;
        bool ownsMemory = unit->m_ownsMemory;
        unit->~IOUnit();
        IOUnitContainer::Free(unit, ownsMemory);
        unit = next;
    }

    return result;
}

// VERIFY: 361b842f auto
// OFFSET: 0x0042B980
IORangeUnit::IORangeUnit(int64_t start, int64_t end, bool ownsMemory, IOUnit* next) : IOUnit(next, ownsMemory) {
    this->m_start = start;
    this->m_end = end;
}

// VERIFY: f3b88e5a auto
// OFFSET: 0x004295F0
uint32_t IORangeUnit::GetType() {
    return 'rang';
}

// VERIFY: 9233e4e6 manual
// OFFSET: 0x00426360
bool IORangeUnit::Clamp(int64_t* offset, uint32_t* count, uint32_t* bytesRead) {
    if (*offset < 0) {
        return false;
    }

    *offset += this->m_start;

    if (*offset + *count > this->m_end) {
        if (*offset >= this->m_end || !bytesRead) {
            return false;
        }

        *count = static_cast<uint32_t>(this->m_end - *offset);
    }

    return true;
}

// VERIFY: 6eb04689 auto
// OFFSET: 0x004263E0
bool IORangeUnit::Read(void* buffer, int64_t offset, uint32_t count, uint32_t* bytesRead) {
    if (!this->Clamp(&offset, &count, bytesRead)) {
        return false;
    }

    return this->m_next->Read(buffer, offset, count, bytesRead);
}

// VERIFY: 8e83cccc manual
// OFFSET: 0x00426430
bool IORangeUnit::Write(const void* buffer, int64_t offset, uint32_t count) {
    if (offset < 0) {
        return false;
    }

    int64_t position = this->m_start + offset;

    if (position + count > this->m_end) {
        return false;
    }

    return this->m_next->Write(buffer, position, count);
}

// VERIFY: dd2344d9 manual
// OFFSET: 0x00426490
bool IORangeUnit::GetFileSize(int64_t* size) {
    if (!this->m_next->GetFileSize(size)) {
        return false;
    }

    if (*size <= this->m_start) {
        *size = 0;
    } else if (*size < this->m_end) {
        *size -= this->m_start;
    } else {
        *size = this->m_end - this->m_start;
    }

    return true;
}

// VERIFY: 62c22cb6 auto
// OFFSET: 0x00426520
bool IORangeUnit::SetFileSize(int64_t size) {
    return size == this->m_end - this->m_start;
}

// VERIFY: 3421d7da manual
// OFFSET: 0x0042FAE0
IOLockUnit* IOLockUnit::Create(IOUnitContainer* container, uint32_t flags, IOUnit* next) {
    if (!next) {
        return nullptr;
    }

    bool ownsMemory;
    void* memory = IOUnitContainer::Alloc(container, sizeof(IOLockUnit), &ownsMemory);

    if (!memory) {
        return nullptr;
    }

    return new (memory) IOLockUnit(next, ownsMemory);
}

// VERIFY: d0d6cdac auto
// OFFSET: 0x00430BC0
IOLockUnit* IOLockUnit::Create(IOUnitContainer* container, IOUnit* next) {
    return IOLockUnit::Create(container, 0, next);
}

// INLINED
IOLockUnit::IOLockUnit(IOUnit* next, bool ownsMemory) : IOUnit(next, ownsMemory) {
    this->m_mutex = &this->m_ownMutex;
    Blizzard::Lock::MutexCreate(this->m_ownMutex);
}

// VERIFY: 8706d800 auto
// OFFSET: 0x0042B6D0
IOLockUnit::~IOLockUnit() {
    Blizzard::Lock::MutexDestroy(this->m_ownMutex);
}

// VERIFY: d4a9ac17 auto
// OFFSET: 0x00429600
uint32_t IOLockUnit::GetType() {
    return 'lock';
}

// VERIFY: cf5bd377 manual
// OFFSET: 0x0042B710
bool IOLockUnit::Read(void* buffer, int64_t offset, uint32_t count, uint32_t* bytesRead) {
    Blizzard::Lock::ScopedMutex lock(*this->m_mutex);
    return this->m_next->Read(buffer, offset, count, bytesRead);
}

// VERIFY: 4f9022ee manual
// OFFSET: 0x0042B780
bool IOLockUnit::Write(const void* buffer, int64_t offset, uint32_t count) {
    Blizzard::Lock::ScopedMutex lock(*this->m_mutex);
    return this->m_next->Write(buffer, offset, count);
}

// VERIFY: 30cb338b manual
// OFFSET: 0x0042B7F0
bool IOLockUnit::Flush() {
    Blizzard::Lock::ScopedMutex lock(*this->m_mutex);
    return this->m_next->Flush();
}

// VERIFY: 0f1c67e2 manual
// OFFSET: 0x0042B850
bool IOLockUnit::GetFileSize(int64_t* size) {
    Blizzard::Lock::ScopedMutex lock(*this->m_mutex);
    return this->m_next->GetFileSize(size);
}

// VERIFY: f3a625dc manual
// OFFSET: 0x0042B8B0
bool IOLockUnit::SetFileSize(int64_t size) {
    Blizzard::Lock::ScopedMutex lock(*this->m_mutex);
    return this->m_next->SetFileSize(size);
}

// VERIFY: d9248c56 manual
// OFFSET: 0x0042B920
void* IOLockUnit::Win32Handle() {
    Blizzard::Lock::ScopedMutex lock(*this->m_mutex);
    return this->m_next->Win32Handle();
}

// VERIFY: 4926450f manual
// OFFSET: 0x00435050
IORangeUnit* IORangeUnit::Create(IOUnitContainer* container, int64_t start, int64_t end, IOUnit* next) {
    if (!next) {
        return nullptr;
    }

    if (start > end) {
        next->Release();
        return nullptr;
    }

    bool ownsMemory;
    void* memory = IOUnitContainer::Alloc(container, sizeof(IORangeUnit), &ownsMemory);

    if (!memory) {
        return nullptr;
    }

    return new (memory) IORangeUnit(start, end, ownsMemory, next);
}

// VERIFY: dfd9ee20 manual
// OFFSET: 0x00429A20
bool IOLockUnit::ReplaceNext(IOUnit* lock, IOUnit* next) {
    if (!lock || !next || lock->GetType() != 'lock' || next->m_next != lock->m_next) {
        return false;
    }

    lock->m_next = next;

    return true;
}
