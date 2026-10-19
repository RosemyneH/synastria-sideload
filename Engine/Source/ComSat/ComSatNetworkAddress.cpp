#include "Engine/Source/ComSat/ComSatNetwork.h"
#include <cstring>

// VERIFY: 86ec6037 auto
// OFFSET: 0x00885CB0
ComSat::Network::Address::Address() {
    this->m_length = 16;
    this->m_family = 0;
    this->m_port = 0;
    this->m_ip = 0;
    memset(this->m_zero, 0, sizeof(this->m_zero));
    this->m_family = 2;
}

// VERIFY: 62366b28 auto
// OFFSET: 0x00885C70
bool ComSat::Network::Address::operator==(const ComSat::Network::Address& other) const {
    return this->m_length == other.m_length && this->m_family == other.m_family && this->m_port == other.m_port && this->m_ip == other.m_ip;
}
