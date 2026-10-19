#include "IOFileUnit.h"

namespace Storm {

// VERIFY: 5941ff82 manual
// OFFSET: 0x006B9DD0
void IORefcount::Release() {
    IORefcount* refcount = this;

    do {
        IORefcount* next = refcount->m_refcount;

        if (--refcount->m_count == 0) {
            delete refcount;
        }

        refcount = next;
    } while (refcount);
}

}
