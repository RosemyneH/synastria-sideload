#ifndef ENGINE_SOURCE_STEELSERIESMOUSE_WOWMOUSE_H
#define ENGINE_SOURCE_STEELSERIESMOUSE_WOWMOUSE_H

#include <cstddef>
#include <cstdint>

class HidManager;

class SteelseriesMouse {
    public:
    virtual ~SteelseriesMouse();
    virtual int32_t IsConnected() = 0;
};

class WowMouse : public SteelseriesMouse {
    public:
    /* 0004 */ HidManager* m_hidManager;

    WowMouse();
    ~WowMouse() override;
    int32_t IsConnected() override;
};

#endif
