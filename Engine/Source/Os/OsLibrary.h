#ifndef ENGINE_SOURCE_OS_OSLIBRARY_H
#define ENGINE_SOURCE_OS_OSLIBRARY_H

void OsFreeLibrary(void* module);
void* OsGetLibraryEntryPoint(void* module, const char* name);
void* OsLoadLibrary(const char* fileName);

#endif
