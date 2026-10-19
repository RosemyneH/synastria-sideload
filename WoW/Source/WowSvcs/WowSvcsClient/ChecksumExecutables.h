#ifndef WOW_SOURCE_WOWSVCS_WOWSVCSCLIENT_CHECKSUMEXECUTABLES_H
#define WOW_SOURCE_WOWSVCS_WOWSVCSCLIENT_CHECKSUMEXECUTABLES_H

#include <cstdint>

void ChecksumExecutables(const uint8_t* challenge, uint32_t challengeLength, uint8_t* checksum);

#endif
