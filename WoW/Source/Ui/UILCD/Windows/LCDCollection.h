#ifndef WOW_SOURCE_UI_UILCD_WINDOWS_LCDCOLLECTION_H
#define WOW_SOURCE_UI_UILCD_WINDOWS_LCDCOLLECTION_H

#include <cstdint>
#include "Engine/Source/Storm/H/stpl.h"
#include "WoW/Source/Ui/UILCD/Windows/LCDBase.h"

struct CLCDBASE_NODE : public TSLinkedNode<CLCDBASE_NODE> {
    /* 0008 */ CLCDBase* m_pObject;
};

class CLCDCollection : public CLCDBase {
    public:
    /* 0040 */ TSList<CLCDBASE_NODE, TSGetLink<CLCDBASE_NODE> > m_Objects;

    CLCDCollection();
    virtual ~CLCDCollection();
    virtual void OnDraw(CLCDGfxBase& gfx);
    virtual void OnUpdate(uint32_t timestamp);
    virtual void ShowSubscreen(int32_t subscreen);

    bool AddObject(CLCDBase* object);
};

#endif
