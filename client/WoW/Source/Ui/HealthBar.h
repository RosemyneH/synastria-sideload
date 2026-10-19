#ifndef WOW_SOURCE_UI_HEALTHBAR_H
#define WOW_SOURCE_UI_HEALTHBAR_H

#include <cstddef>
#include <cstdint>
#include "Engine/Source/Base/CDataAllocator.h"
#include "Engine/Source/Frame/CSimpleStatusBar.h"
#include "WoW/Source/Object/WGUID.h"

class CGUnit_C;
class CImVector;
class CSimpleFrame;

class CGSimpleHealthBar : public CSimpleStatusBar {
    public:
    static CDataAllocator s_simpleHealthBarHeap;

    static void operator delete(void* ptr);

    /* 02C8 */ WOWGUID m_guid;
    /* 02D0 */ uint32_t m_autoColor;

    ~CGSimpleHealthBar() override;
    void SetValue(float value) override;
    void SetStatusBarColor(const CImVector& color) override;

    CGSimpleHealthBar(CSimpleFrame* parent);
    void InstallMirrorHandlers();
    void RemoveMirrorHandlers();
    void SetUnit(CGUnit_C* unit);
};

#endif
