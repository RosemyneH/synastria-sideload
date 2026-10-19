#ifndef ENGINE_SOURCE_COMSAT_COMSATSOUNDCODEC_H
#define ENGINE_SOURCE_COMSAT_COMSATSOUNDCODEC_H

#include <cstddef>
#include <cstdint>
#include "Engine/Source/ComSat/ComSatInterface.h"
#include "Engine/Source/ComSat/ComSatMap.h"
#include "Engine/Source/ComSat/SPIRIT/ipmr.h"

namespace ComSat {
namespace Sound {

class CODEC {
    public:
    static CODEC* Create(const Initialization& init, Allocator& allocator);
    static bool CanBitpackFrames();
    static bool IsEncodedBitrateVariable();
    static uint32_t MinimumSafeFramesBetweenResets();

    /* 0004 */ Allocator* m_allocator;

    virtual ~CODEC();
    virtual bool Init();
    virtual bool Reset();
    virtual void Shutdown();
    virtual bool InitLocalPlayer(uint8_t player, float threshold) = 0;
    virtual bool RemoveLocalPlayer(uint8_t player) = 0;
    virtual void ResetPlayerChannelEncoder(uint8_t player) = 0;
    virtual void ResetPlayerChannelDecoder(uint8_t player, int32_t channel) = 0;
    virtual bool CompressRawData(uint8_t player, const uint8_t* pcm, uint32_t size, uint8_t* data, uint32_t* bits, bool* voice) = 0;
    virtual bool DecompressEncodedData(uint8_t player, int32_t channel, const uint8_t* data, uint32_t bits, uint8_t* pcm, uint32_t* size) = 0;
    virtual void SetThreshold(uint8_t player, float threshold) = 0;
    virtual void GenerateSilence(uint8_t* pcm, uint32_t size) = 0;
    virtual void ApplyVoiceEffects(uint8_t player, uint8_t* pcm, uint32_t* size) = 0;

    CODEC(Allocator& allocator);
};

class CODECIPMR6000 : public CODEC {
    public:
    class LocalPlayer {
        public:
        /* 0004 */ Allocator* m_allocator;
        /* 0008 */ int32_t m_channelCount;
        /* 000C */ uint8_t m_encoder[sizeof(SPIRIT_IPMR_STATE) + 16];
        /* 34CC */ uint8_t** m_decoders;
        /* 34D0 */ uint8_t* m_tables;
        /* 34D4 */ int32_t m_hangover;
        /* 34D8 */ float m_threshold;

        virtual ~LocalPlayer();

        LocalPlayer(Allocator& allocator, int32_t channelCount, uint8_t* tables);
        bool CompressRawData(const uint8_t* pcm, uint32_t size, uint8_t* data, uint32_t* bits, bool* voice);
        bool DecompressEncodedData(int32_t channel, const uint8_t* data, uint32_t bits, uint8_t* pcm, uint32_t* size);
    };

    static const uint8_t s_invalidPlayer;
    static const int32_t s_maxFramesPerPacket;
    static const int32_t s_sampleRate;
    static const int32_t s_bitsPerSample;
    static const int32_t s_samplesPerFrame;
    static const int32_t s_minEncodedFrameLength;
    static const int32_t s_maxEncodedFrameLength;
    static const uint32_t s_bytesPerFrame;

    /* 0008 */ bool m_initialized;
    /* 000C */ uint32_t m_maxChannels;
    /* 0010 */ uint8_t m_tables[sizeof(SPIRIT_IPMR_TABLES) + 16];
    /* BBF0 */ Map<uint8_t, LocalPlayer*> m_players;

    virtual ~CODECIPMR6000();
    virtual bool Init();
    virtual bool Reset();
    virtual void Shutdown();
    virtual bool InitLocalPlayer(uint8_t player, float threshold);
    virtual bool RemoveLocalPlayer(uint8_t player);
    virtual void ResetPlayerChannelEncoder(uint8_t player);
    virtual void ResetPlayerChannelDecoder(uint8_t player, int32_t channel);
    virtual bool CompressRawData(uint8_t player, const uint8_t* pcm, uint32_t size, uint8_t* data, uint32_t* bits, bool* voice);
    virtual bool DecompressEncodedData(uint8_t player, int32_t channel, const uint8_t* data, uint32_t bits, uint8_t* pcm, uint32_t* size);
    virtual void SetThreshold(uint8_t player, float threshold);
    virtual void GenerateSilence(uint8_t* pcm, uint32_t size);
    virtual void ApplyVoiceEffects(uint8_t player, uint8_t* pcm, uint32_t* size);

    CODECIPMR6000(const Initialization& init, Allocator& allocator);
};

}
}

#endif
