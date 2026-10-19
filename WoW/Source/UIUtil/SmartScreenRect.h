#ifndef WOW_SOURCE_UIUTIL_SMARTSCREENRECT_H
#define WOW_SOURCE_UIUTIL_SMARTSCREENRECT_H

#include <cstdint>

class C3Vector;
class CRect;
class CSimpleFrame;

void SmartScreenRectClearAllGrids();
void SmartScreenRectShutdown();
void SmartScreenRectClearGrid(uint32_t grid);
void SmartScreenRectPlace(uint32_t grid, CRect& rect);
void SmartScreenRectPositionFrame(uint32_t grid, CSimpleFrame* frame, CSimpleFrame* parent, const C3Vector& position, int32_t center);

#endif
