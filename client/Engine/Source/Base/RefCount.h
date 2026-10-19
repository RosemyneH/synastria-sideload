#ifndef ENGINE_SOURCE_BASE_REFCOUNT_H
#define ENGINE_SOURCE_BASE_REFCOUNT_H

#include <cstdint>

class TRefCnt {
    public:
    /* 0004 */ uint32_t m_refcnt;

    TRefCnt();
    virtual void DeleteSelf();
    virtual ~TRefCnt();
};

#endif
