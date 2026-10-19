#ifndef ENGINE_SOURCE_COMSAT_COMSATINTERFACE_H
#define ENGINE_SOURCE_COMSAT_COMSATINTERFACE_H

#include <cstddef>
#include <cstdint>
#include "Engine/Source/ComSat/Types.h"

namespace ComSat {

class Allocator {
    public:
    virtual ~Allocator();
    virtual void* Alloc(uint32_t size) = 0;
    virtual void Free(void* ptr) = 0;
};

struct DeviceInfo {
    /* 0000 */ char m_name[256];
};

struct Initialization {
    /* 0000 */ uint32_t m_maxLocalPlayers;
    /* 0004 */ uint32_t unk_0004;
    /* 0008 */ void* m_window;
    /* 000C */ uint16_t m_port;
    /* 0010 */ uint32_t m_time;
    /* 0014 */ uint32_t m_maxChannels;
};

class Interface {
    public:
    class Callback {
        public:
        virtual ~Callback();
        virtual void NotifyLocalTalkerStart(uint8_t localTalker) = 0;
        virtual void NotifyLocalTalkerStop(uint8_t localTalker) = 0;
        virtual void NotifyTalkerStart(uint8_t localTalker, uint64_t session, uint64_t talker) = 0;
        virtual void NotifyTalkerStop(uint8_t localTalker, uint64_t session, uint64_t talker) = 0;
        virtual void NotifyLocalTalkerVoiceLevel(uint8_t localTalker, float level, bool talking) = 0;
        virtual void NotifyDeviceError(uint8_t localTalker) = 0;
    };

    static Interface* Create(const Initialization& init, Allocator& allocator);
    static void Destroy(Interface* comsat, Allocator& allocator);

    virtual ~Interface();
    virtual void Work(uint32_t elapsed) = 0;
    virtual int32_t EnumerateOutputDevices(DeviceInfo* devices, int32_t maxDevices) = 0;
    virtual int32_t EnumerateCaptureDevices(DeviceInfo* devices, int32_t maxDevices) = 0;
    virtual void CallbackSet(Interface::Callback& callback) = 0;
    virtual void CallbackClear() = 0;
    virtual bool SessionCreate(uint64_t session) = 0;
    virtual bool SessionDestroy(uint64_t session) = 0;
    virtual bool SessionSetNetworkId(uint64_t session, uint16_t networkId) = 0;
    virtual bool SessionAddLocal(uint64_t session, uint8_t localTalker, uint64_t talker) = 0;
    virtual bool SessionAddRemote(uint64_t session, uint64_t talker, uint8_t networkId, const uint8_t (&key)[16], const char* address) = 0;
    virtual bool SessionSetTalkerNetworkIdLocal(uint64_t session, uint64_t talker, uint8_t networkId) = 0;
    virtual bool SessionSetTalkerNetworkIdRemote(uint64_t session, uint64_t talker, uint8_t networkId) = 0;
    virtual bool SessionRemove(uint64_t session, uint64_t talker) = 0;
    virtual int32_t SessionNumMembers(uint64_t session) = 0;
    virtual bool SessionGetTalkerByIndex(uint64_t session, int32_t index, uint64_t& talker, uint8_t& localTalker) = 0;
    virtual bool SessionSetTalkerPriority(uint64_t session, uint64_t talker, uint8_t priority) = 0;
    virtual bool LocalTalkerCreate(uint8_t localTalker, uint64_t talker, float threshold, int32_t outputDevice, int32_t captureDevice) = 0;
    virtual bool LocalTalkerDestroy(uint8_t localTalker) = 0;
    virtual bool LocalTalkerChangeOutputDevice(uint8_t localTalker, int32_t device) = 0;
    virtual bool LocalTalkerChangeCaptureDevice(uint8_t localTalker, int32_t device) = 0;
    virtual int32_t LocalTalkerGetCurrentOutputDeviceIndex(uint8_t localTalker) = 0;
    virtual int32_t LocalTalkerGetCurrentCaptureDeviceIndex(uint8_t localTalker) = 0;
    virtual bool LocalTalkerEnable(uint8_t localTalker, bool enable) = 0;
    virtual bool LocalTalkerThreshold(uint8_t localTalker, float threshold) = 0;
    virtual bool LocalTalkerEnableVoiceLevelCallback(uint8_t localTalker, bool enable) = 0;
    virtual bool LocalTalkerSelectSession(uint8_t localTalker, uint64_t session) = 0;
    virtual bool LocalTalkerSetSessionPriority(uint8_t localTalker, uint64_t session, uint8_t priority) = 0;
    virtual bool LocalTalkerSetRemoteTalkerVolume(uint8_t localTalker, uint64_t talker, float volume) = 0;
    virtual bool LocalTalkerSetSessionVolume(uint8_t localTalker, uint64_t session, float volume) = 0;
    virtual bool MuteTalker(uint8_t localTalker, uint64_t talker, MuteSetting setting) = 0;
    virtual bool IsTalkerMuted(uint8_t localTalker, uint64_t talker) = 0;
    virtual bool MuteSession(uint8_t localTalker, uint64_t session, MuteSetting setting) = 0;
    virtual bool IsSessionMuted(uint8_t localTalker, uint64_t session) = 0;
    virtual bool IsTalkerTalkingInSession(uint64_t session, uint8_t localTalker, uint64_t talker) = 0;
    virtual bool DoesTalkerHaveHeadset(uint64_t talker) = 0;
    virtual bool DoesLocalPlayerHaveHeadset(uint8_t localTalker) = 0;
    virtual void DebugSetSequenceDelay(int32_t delay) = 0;
    virtual void DebugSetSoundOutputDelay(uint8_t localTalker, int32_t delay) = 0;
};

}

#endif
