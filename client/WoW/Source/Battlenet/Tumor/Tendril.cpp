#include "WoW/Source/Battlenet/Tumor/Tendril.h"
#include "WoW/Source/Battlenet/Tumor/TumorManager.h"

// INLINED
Battlenet::Service::Interface::Interface(TumorManager& manager) {
    this->m_manager = &manager;
    this->m_eventAdapter = nullptr;
    this->m_tendrilId = 0;
}

// VERIFY: 71c346c6 auto
// OFFSET: 0x00653A10
void Battlenet::Service::Interface::RecvEvent(const Event::Header& header, const BSN::EncodedObject* data) {
}

// VERIFY: d97051ab manual
// OFFSET: 0x009C1950
Battlenet::Service::Interface::~Interface() {
    if (this->m_tendrilId) {
        this->m_manager->GetTumor()->RemoveTendril(this->m_tendrilId);
    }
}

// VERIFY: 0d162034 auto
// OFFSET: 0x009C1900
void Battlenet::Service::Interface::Register() {
    if (this->m_eventId.m_value) {
        Tumor* tumor = this->m_manager->GetTumor();
        tumor->AllocTendrilId(this->m_tendrilId);
        this->m_eventAdapter = tumor->AllocEventAdapter(this->m_eventId, *this, this->m_tendrilId);
        tumor->Subscribe(this->m_eventId, *this, this->m_tendrilId);
    }
}

// VERIFY: ba55f5d0 manual
// OFFSET: 0x009C1AE0
void Battlenet::Service::Interface::SendEvent(const uint8_t& eventType, const BSN::FourCC& eventId, const BSN::EncodedObject* data) {
    if (this->m_eventAdapter) {
        this->m_eventAdapter->SendEvent(eventType, eventId, data);
    }
}

// VERIFY: b8093540 manual
// OFFSET: 0x009C19A0
Battlenet::Tendril::Tendril(BSN::FourCC eventId, TumorManager& manager) : Service::Interface(manager) {
    this->m_eventId = eventId;

    if (this->m_eventId.m_value) {
        Tumor* tumor = this->m_manager->GetTumor();
        tumor->AllocTendrilId(this->m_tendrilId);
        this->m_eventAdapter = tumor->AllocEventAdapter(this->m_eventId, *this, this->m_tendrilId);
        tumor->Subscribe(this->m_eventId, *this, this->m_tendrilId);
    }

    this->m_adapter = manager.GetTumor()->AllocAdapter(*this, this->m_tendrilId);
}

// OFFSET: 0x009C1C60
Battlenet::Tendril::~Tendril() {
}

// VERIFY: 84c27b8e auto
// OFFSET: 0x005EEB70
void Battlenet::Tendril::DoSubscriptions() {
}
