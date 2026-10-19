#ifndef ENGINE_SOURCE_OS_OSCLIPBOARD_H
#define ENGINE_SOURCE_OS_OSCLIPBOARD_H

#include <cstdint>

char* IOsClipboardGetString(void* window);
int32_t IOsClipboardPutString(const char* string, void* window);
char* OsClipboardGetString();
int32_t OsClipboardPutString(const char* string);
void OsClipboardFreeString(char* string);

#endif
