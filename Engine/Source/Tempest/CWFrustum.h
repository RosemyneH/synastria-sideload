#ifndef ENGINE_SOURCE_TEMPEST_CWFRUSTUM_H
#define ENGINE_SOURCE_TEMPEST_CWFRUSTUM_H

#include <cstddef>
#include <cstdint>
#include "Engine/Source/Tempest/CFrustum.h"
#include "Engine/Source/Storm/H/stpl.h"

class CWFrustum : public CFrustum {
    public:
    /* 00F4 */ TSLink<CWFrustum> m_link;

    CWFrustum();
    CWFrustum(const C3Vector* corners);
    CWFrustum(const CFrustum& frustum);
};

// INLINED
inline CWFrustum::CWFrustum() {
}

// INLINED
inline CWFrustum::CWFrustum(const C3Vector* corners) : CFrustum(corners) {
}

// INLINED
inline CWFrustum::CWFrustum(const CFrustum& frustum) : CFrustum(frustum) {
}

#endif
