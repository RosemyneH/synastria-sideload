#ifndef ENGINE_SOURCE_COMSAT_COMSATSESSION_H
#define ENGINE_SOURCE_COMSAT_COMSATSESSION_H

#include <cstddef>
#include <cstdint>
#include "Engine/Source/ComSat/ComSatInterface.h"
#include "Engine/Source/ComSat/ComSatNetwork.h"

namespace ComSat {

class SessionCallback {
    public:
    virtual ~SessionCallback();
    virtual void IncomingVoiceData(uint64_t session, uint64_t talker, const uint8_t* data, uint32_t size) = 0;
};

class Session : public Network::Session::Callback {
    public:
    /* 0004 */ SessionCallback* m_callback;
    /* 0008 */ Network::Session* m_networkSession;
    /* 0010 */ uint64_t m_id;
    /* 0018 */ Allocator* m_allocator;

    virtual ~Session();
    virtual void Recv(uint64_t talker, const uint8_t* data, uint32_t size);

    Session(SessionCallback& callback, uint64_t id, Allocator& allocator);
    bool Add(uint64_t talker, uint8_t networkId, const uint8_t (&key)[16], const Network::Address& address);
    Network::Session* GetNetworkSession();
    bool LocalAdd(uint64_t talker);
    void Remove(uint64_t talker);
    bool Send(uint64_t talker, const uint8_t* data, uint32_t size);
    void SetNetworkSession(Network::Session* session);
};

}

#endif
