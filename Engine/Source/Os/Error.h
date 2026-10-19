#ifndef ENGINE_SOURCE_OS_ERROR_H
#define ENGINE_SOURCE_OS_ERROR_H

char* OsGetLastErrorStr();
void OsFreeLastErrorStr(char* str);

#endif
