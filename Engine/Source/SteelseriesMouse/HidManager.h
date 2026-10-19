#ifndef ENGINE_SOURCE_STEELSERIESMOUSE_HIDMANAGER_H
#define ENGINE_SOURCE_STEELSERIESMOUSE_HIDMANAGER_H

#include <cstddef>
#include <cstdint>

class HidManager {
    public:
    static HidManager* Create();

    virtual ~HidManager() = 0;
    virtual bool EnumerateDevices() = 0;
    virtual void SetWindow(void* window) = 0;
    virtual uint32_t GetDeviceCount() = 0;
    virtual bool WriteWait(const void* data, uint32_t size) = 0;
    virtual bool Write(const void* data, uint32_t size) = 0;
    virtual bool SetFeature(void* data, uint32_t size) = 0;
};

#endif
