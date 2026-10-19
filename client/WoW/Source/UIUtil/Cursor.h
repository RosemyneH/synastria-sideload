#ifndef WOW_SOURCE_UIUTIL_CURSOR_H
#define WOW_SOURCE_UIUTIL_CURSOR_H

#include <cstdint>
#include "WoW/Source/Object/WGUID.h"

struct MipBits;

int32_t CopyCursorImage(uint32_t* image, MipBits* mipImages, uint32_t width, uint32_t height);
void CursorInitialize();
void CursorDestroy();

void CursorReleaseItemMips();
void CursorSetHeldItem(WOWGUID item);
uint32_t CursorGetResetMode();
void CursorResetMode();
void CursorSetMode(uint32_t mode);
void CursorSetResetMode(uint32_t mode);
uint32_t CursorGetModeFromName(const char* name);
int32_t CursorSetFromName(const char* name);
void CursorDropItem();
void UpdateCursor();
void CursorGrabMoney(uint32_t amount);
void CursorGrabSpell(const char* texture);
void CursorGrabVirtualItem(int32_t displayId);

#endif
