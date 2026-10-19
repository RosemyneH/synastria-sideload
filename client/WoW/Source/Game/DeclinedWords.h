#ifndef WOW_SOURCE_GAME_DECLINEDWORDS_H
#define WOW_SOURCE_GAME_DECLINEDWORDS_H

#include <cstdint>
#include "Engine/Source/Storm/H/stpl.h"

struct DECLINEDWORD : public TSHashObject<DECLINEDWORD, HASHKEY_CONSTSTR> {
    /* 0018 */ int32_t m_wordId;
    /* 001C */ int32_t m_caseIndex;
};

void DeclinedWordsDestroy();
void DeclinedWordsInitialize();

#endif
