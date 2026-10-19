#ifndef WOW_SOURCE_SOUNDINTERFACE_SOUNDINTERFACE2DSP_H
#define WOW_SOURCE_SOUNDINTERFACE_SOUNDINTERFACE2DSP_H

#include <cstddef>
#include <cstdint>
#include "Engine/Source/SoundEngine/fmodex/src/fmod.hpp"
#include "Engine/Source/Storm/H/stpl.h"
#include "WoW/Source/SoundInterface/Types.h"

class SI2DspNode : public TSLinkedNode<SI2DspNode> {
    public:
    /* 0008 */ FMOD::DSP* m_dsp;
    /* 000C */ int32_t m_type;

    SI2DspNode();
};

class SI2DspChainHead {
    public:
    /* 0000 */ TSList<SI2DspNode, TSGetLink<SI2DspNode> > m_nodes;
    /* 000C */ FMOD::ChannelGroup* m_channelGroup;
    /* 0010 */ float m_expireTime;

    SI2DspChainHead();
};

struct DSP_CHAIN_HEAD_LOOKUP : public TSHashObject<DSP_CHAIN_HEAD_LOOKUP, HASHKEY_STRI> {
    /* 0018 */ SI2DspChainHead m_chain;
};

#endif
