#ifndef WOW_SOURCE_WOWSERVICES_SRP6_CLIENT_H
#define WOW_SOURCE_WOWSERVICES_SRP6_CLIENT_H

#include <cstddef>
#include <cstdint>
#include "Engine/Source/BlizzardCore/Source/Packages/Util/SHA1.h"

class SRP6_Random;

class SRP6_Client {
    public:
    /* 0000 */ uint8_t m_publicKey [32];
    /* 0020 */ uint8_t m_sessionKey [40];
    /* 0048 */ uint8_t m_clientProof [20];
    /* 005C */ uint8_t m_accountNameHash [20];
    /* 0070 */ uint8_t m_credentialsHash [20];
    /* 0084 */ SHA1_CONTEXT m_proofContext;

    int32_t BeginAuthentication(const char* accountName, const char* password);
    int32_t CalculateProof(const uint8_t* largeSafePrime, uint32_t largeSafePrimeLength, const uint8_t* generator, uint32_t generatorLength, const uint8_t* salt, uint32_t saltLength, const uint8_t* serverPublicKey, uint32_t serverPublicKeyLength, SRP6_Random& random);
    int32_t VerifyServerProof(const uint8_t* serverProof, uint32_t serverProofLength);
};

#endif
