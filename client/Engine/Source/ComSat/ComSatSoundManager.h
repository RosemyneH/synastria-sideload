#ifndef ENGINE_SOURCE_COMSAT_COMSATSOUNDMANAGER_H
#define ENGINE_SOURCE_COMSAT_COMSATSOUNDMANAGER_H

#include <cstddef>
#include <cstdint>
#include "Engine/Source/ComSat/ComSatInterface.h"
#include "Engine/Source/ComSat/ComSatMap.h"
#include "Engine/Source/ComSat/ComSatRunningAverageCalculator.h"
#include "Engine/Source/ComSat/ComSatSoundCODEC.h"
#include "Engine/Source/ComSat/ComSatSoundIOSoundEngine.h"
#include "Engine/Source/ComSat/ComSatVector.h"

namespace ComSat {
namespace Sound {

class Manager : public IOCallback {
    public:
    class Callback {
        public:
        virtual ~Callback();
        virtual void MicrophoneStatusUpdate(uint8_t player, HeadsetDeviceStatus status) = 0;
        virtual void LocalChatDataReady(uint8_t player, uint8_t* data, uint32_t size, uint32_t time) = 0;
        virtual void RemotePlayerTalkNotifyStart(uint8_t player, uint64_t talker, uint64_t session) = 0;
        virtual void RemotePlayerTalkNotifyStop(uint8_t player, uint64_t talker, uint64_t session) = 0;
        virtual void LocalTalkerVoiceLevel(uint8_t player, float level, bool talking) = 0;
        virtual void DeviceError(uint8_t player) = 0;
    };

    struct TalkingTalkerInfo {
        /* 0000 */ uint64_t m_talker;
        /* 0008 */ uint64_t m_session;
        /* 0010 */ int32_t m_holdFrames;
        /* 0014 */ bool m_active;
    };

    struct VoicePacket {
        /* 0000 */ uint16_t m_sequence;
        /* 0002 */ uint16_t m_bits;
        /* 0004 */ uint8_t m_data[128];
        /* 0084 */ bool m_valid;
        /* 0088 */ uint64_t m_session;
    };

    class LocalPlayer {
        public:
        /* 0000 */ Map<uint64_t, uint8_t> m_sessionPriorities;
        /* 0010 */ Vector<uint64_t> m_mutedSessions;
        /* 0020 */ Vector<uint64_t> m_mutedTalkers;
        /* 0030 */ Map<uint64_t, float> m_talkerVolumes;
        /* 0040 */ Map<uint64_t, float> m_sessionVolumes;
        /* 0050 */ int32_t m_sequence;
        /* 0054 */ uint8_t m_buffer[2048];
        /* 0854 */ bool m_enabled;
        /* 0855 */ bool m_talking;
        /* 0856 */ bool m_resetEncoder;
        /* 0858 */ TalkingTalkerInfo m_channels[4];
        /* 08B8 */ uint16_t m_lastResetSequence;
        /* 08BA */ bool m_voiceLevelCallback;
        /* 08BB */ bool m_deviceError;

        LocalPlayer(Allocator& allocator);
        ~LocalPlayer();
    };

    class RemoteTalker {
        public:
        /* 0008 */ Allocator* m_allocator;
        /* 000C */ VoicePacket* m_packets;
        /* 0010 */ double m_playCursor;
        /* 0018 */ RunningAverageCalculator m_frameTime;
        /* 0038 */ uint32_t m_lastPacketTime;
        /* 003C */ uint16_t m_lastSequence;
        /* 003E */ uint16_t m_lastResetSequence;
        /* 0040 */ uint32_t m_recenterTime;
        /* 0048 */ uint64_t m_guid;

        virtual ~RemoteTalker();

        RemoteTalker(Allocator& allocator);
    };

    class Session {
        public:
        /* 0000 */ uint64_t m_id;
        /* 0008 */ Map<uint64_t, uint8_t> m_talkerPriorities;

        Session(Allocator& allocator);
    };

    /* 0004 */ uint32_t m_maxLocalPlayers;
    /* 0008 */ Allocator* m_allocator;
    /* 000C */ Callback* m_callback;
    /* 0010 */ CODEC* m_codec;
    /* 0014 */ IO* m_io;
    /* 0018 */ Map<uint8_t, LocalPlayer*> m_localPlayers;
    /* 0028 */ Map<uint64_t, Session*> m_sessions;
    /* 0038 */ Map<uint64_t, RemoteTalker*> m_remoteTalkers;
    /* 0048 */ int32_t m_jitterDelay;
    /* 004C */ int32_t m_sequenceDelay;
    /* 0050 */ int32_t m_holdFrames;
    /* 0054 */ float m_framesPerSecond;
    /* 0058 */ uint32_t m_maxChannels;
    /* 005C */ int32_t m_currentFrame;
    /* 0060 */ float m_frameDuration;
    /* 0064 */ uint32_t m_startTime;
    /* 0068 */ uint32_t m_currentTime;

    virtual ~Manager();
    virtual void DeviceError(uint8_t player);

    Manager(const Initialization& init, Allocator& allocator);
    void AdvancePlayCursorForRemoteTalker(RemoteTalker* talker);
    void AssignTalkersToChannelsForLocalPlayer(uint8_t player, int32_t channelCount, TalkingTalkerInfo* infos);
    bool CanBitpackEncodedData();
    uint32_t CalculateTotalVoicePriorityForRemoteTalkerInSessionForLocalPlayer(uint8_t player, uint64_t talker, uint64_t session);
    void DebugSetSequenceDelay(int32_t delay);
    void DebugSetSoundOutputDelay(uint8_t player, int32_t delay);
    bool DecodeVoicePacket(uint8_t player, int32_t channel, VoicePacket* packet, uint32_t* size);
    bool DestroyLocalPlayer(uint8_t player);
    void EncodeAvailableVoiceData();
    void EncodeAvailableVoiceDataForLocalPlayer(uint8_t player, LocalPlayer* localPlayer);
    int32_t EnumerateCaptureDevices(DeviceInfo* devices, int32_t maxDevices);
    int32_t EnumerateOutputDevices(DeviceInfo* devices, int32_t maxDevices);
    int32_t GetMaxEncodedFrameLength();
    int32_t GetMinEncodedFrameLength();
    void GenerateAndWriteSilenceToHeadsetDeviceBuffer(uint8_t player, int32_t channel, float frames, double rate);
    bool InitLocalPlayer(uint8_t player, float threshold, int32_t outputDevice, int32_t captureDevice);
    bool IsLocalPlayerTalking(uint8_t player);
    bool IsRemoteTalkerMuted(uint8_t player, uint64_t talker);
    bool IsRemoteTalkerTalkingInSession(uint64_t session, uint8_t player, uint64_t talker);
    bool IsSessionMuted(uint8_t player, uint64_t session);
    bool IsUsingVariableBitrateCodec();
    bool LocalTalkerChangeCaptureDevice(uint8_t player, int32_t device);
    bool LocalTalkerChangeOutputDevice(uint8_t player, int32_t device);
    bool LocalTalkerEnable(uint8_t player, bool enable);
    bool LocalTalkerEnableVoiceLevelCallback(uint8_t player, bool enable);
    int32_t LocalTalkerGetCurrentCaptureDeviceIndex(uint8_t player);
    int32_t LocalTalkerGetCurrentOutputDeviceIndex(uint8_t player);
    bool LocalTalkerSetRemoteTalkerVolume(uint8_t player, uint64_t talker, float volume);
    bool LocalTalkerSetSessionVolume(uint8_t player, uint64_t session, float volume);
    bool LocalTalkerThreshold(uint8_t player, float threshold);
    void PlayScheduledVoiceData();
    void PlayVoiceDataForLocalPlayer(uint8_t player, int32_t frames);
    void RecenterJitterBufferForRemoteTalker(uint64_t talker, int32_t offset);
    bool RegisterLocalTalkerForSession(uint64_t session, uint8_t player);
    bool RegisterRemoteTalkerForSession(uint64_t session, uint64_t talker);
    bool SessionCreate(uint64_t session);
    bool SessionDestroy(uint64_t session);
    bool SessionGetTalkerByIndex(uint64_t session, int32_t index, uint64_t& talker, uint8_t& player);
    int32_t SessionNumMembers(uint64_t session);
    void SetCallback(Callback* callback);
    bool SetPlaybackPriorityForPlayerInSession(uint64_t session, uint64_t talker, uint8_t priority);
    bool SetPlaybackPriorityForSession(uint64_t session, uint8_t player, uint8_t priority);
    bool SetSessionMuted(uint8_t player, uint64_t session, MuteSetting setting);
    bool SetTalkerMuted(uint8_t player, uint64_t talker, MuteSetting setting);
    void SubmitIncomingFrameTimeData(uint64_t talker, uint16_t sequence);
    void SubmitIncomingVoicePacket(uint64_t session, uint64_t talker, uint8_t* data, uint32_t size, bool reliable);
    void TransferEncodedVoiceDataForLocalPlayer(uint8_t player, const uint8_t* data, uint32_t bits, uint32_t time);
    bool UnregisterLocalTalkerFromSession(uint64_t session, uint8_t player);
    bool UnregisterRemoteTalkerFromSession(uint64_t session, uint64_t talker);
    void Work(uint32_t time);
    void WriteVoiceDataToHeadsetDeviceBuffer(uint8_t player, int32_t channel, uint32_t size, double rate);
};

}
}

#endif
