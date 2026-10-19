#ifndef WOW_SOURCE_OBJECTALLOC_HEAPUSAGE_H
#define WOW_SOURCE_OBJECTALLOC_HEAPUSAGE_H

#include <cstddef>
#include <cstdint>

struct CVar;

extern CVar* s_cvHeapAllocTracking;

bool ChangeHeapAllocTracking(CVar* var, const char* oldValue, const char* value, void* arg);
void HeapUsageDestroy();
void HeapUsageInitialize();

#endif
