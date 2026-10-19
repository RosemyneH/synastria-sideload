#ifndef WOW_SOURCE_WOWSVCS_WOWSVCSCLIENT_CHECKEXECUTABLESIGNATURE_H
#define WOW_SOURCE_WOWSVCS_WOWSVCSCLIENT_CHECKEXECUTABLESIGNATURE_H

#include <cstdint>

typedef int32_t (__stdcall* SCANDLL_SCANFUNC)(int32_t* type, char* hackName, uint32_t hackNameSize);
typedef int32_t (__stdcall* SCANDLL_SCANFUNC2)(int32_t* type, int32_t* shouldBlockPlaying, char* hackName, uint32_t hackNameSize);

bool CheckExecutableSignature(const char* fileName);
bool GetDllVersionString(const char* fileName, char* buffer, uint32_t size);
bool ScanFileForHack(const char* fileName, uint8_t* hackFound, uint8_t* shouldBlockPlaying, const char** result, char* hackName, uint32_t hackNameSize);

#endif
