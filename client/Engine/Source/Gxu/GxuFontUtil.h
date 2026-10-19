#ifndef ENGINE_SOURCE_GXU_GXUFONTUTIL_H
#define ENGINE_SOURCE_GXU_GXUFONTUTIL_H

#include <cstddef>
#include <cstdint>
#include <freetype/freetype.h>

struct CGxFont;
struct GXUEMBEDDEDTEXTUREINFO;

void CalcWrapPoint(CGxFont* face, const char* text, float fontHeight, float blockWidth, uint32_t* numBytes, float* width, const char** nextText, float shadowWidth, uint32_t flags, bool* wordBreak, float* height, float scale);
uint32_t InternalGetMaxCharsWithinWidth(CGxFont* face, const char* text, float fontHeight, float maxWidth, uint32_t numBytes, float* extent, uint32_t flags, float shadowWidth, float scale, uint32_t* bytesInString, float* widthArray);
void InternalGetTextExtent(CGxFont* face, const char* text, uint32_t numBytes, float fontHeight, float* extent, uint32_t flags, float shadowWidth, float scale);
void* FreeTypeAlloc(FT_Memory memory, long size);
void FreeTypeFree(FT_Memory memory, void* block);
void* FreeTypeRealloc(FT_Memory memory, long curSize, long newSize, void* block);
bool ParseEmbeddedTexture(const char* text, GXUEMBEDDEDTEXTUREINFO* info, float fontPixelHeight, float scale, float fontHeight);
float PixelToScreenHeight(int32_t height);
float ScreenToPixelHeight(int32_t unscaled, float height);
float ScreenToPixelWidth(int32_t unscaled, float width);
bool SetProjection();

#endif
