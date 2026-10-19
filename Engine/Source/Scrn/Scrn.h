#ifndef ENGINE_SOURCE_SCRN_SCRN_H
#define ENGINE_SOURCE_SCRN_SCRN_H

#include <cstdint>
#include "Engine/Source/Scrn/Types.h"
#include "Engine/Source/Services/Types.h"
#include "Engine/Source/Storm/H/Types.h"

uint32_t ScrnLayerGetFlags(HLAYER layer);
void ScrnLayerSetFlags(HLAYER layer, uint32_t flags);
void ScrnLayerSetRect(HLAYER layer, const RECTF* rect);
void ScrnLayerPaint();
void ScrnLayerCreate(const RECTF* rect, float zorder, uint32_t flags, void* param, void (*paintFunc)(void* param, const RECTF* rect, const RECTF* visible, float elapsedSec), HLAYER* layer);

HTEXTFONT ScrnGetStockFont(SCRNSTOCK stockID);
float ScrnGetStockFontHeight(SCRNSTOCK stockID);
void ScrnDestroy();
void ScrnInitialize(int32_t param);
void ScrnSetStockFont(SCRNSTOCK stockID, const char* fontTexturePath);

void ScrnSetScreenshotFormat(const char* format);
void ScrnScreenshot(void (*callback)(int32_t success), uint8_t* data, uint32_t size);
void ScrnSetScreenshotQuality(uint32_t quality);

#endif
