#ifndef WOW_SOURCE_BATTLENET_TUMOR_TENDRIL_H
#define WOW_SOURCE_BATTLENET_TUMOR_TENDRIL_H

#include <cstddef>
#include <cstdint>
#include "WoW/Source/Battlenet/Tumor/Tumor.h"

namespace Battlenet {
class TumorManager;

namespace Service {
class Interface : public EventSubscriber {
    public:
    /* 0008 */ TumorManager* m_manager;
    /* 000C */ EventInterface* m_eventAdapter;
    /* 0010 */ uint32_t m_tendrilId;

    void RecvEvent(const Event::Header& header, const BSN::EncodedObject* data) override;
    ~Interface() override;
    virtual void Destroy() = 0;
    virtual void Register();
    virtual void SendEvent(const uint8_t& eventType, const BSN::FourCC& eventId, const BSN::EncodedObject* data);

    Interface(TumorManager& manager);
};
}

class Tendril : public Service::Interface, public SystemClient {
    public:
    /* 0018 */ Tumor::Adapter* m_adapter;

    ~Tendril() override;
    virtual void DoSubscriptions();

    Tendril(BSN::FourCC eventId, TumorManager& manager);
};
}

#endif
