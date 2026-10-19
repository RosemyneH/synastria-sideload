#include <cstdint>
#include "Engine/Source/Scrn/Layer.h"
#include "Engine/Source/Scrn/Scrn.h"
#include "Engine/Source/Scrn/Stock.h"

// STATIC: 0x00B49A24
static int32_t s_initParam;

// VERIFY: 1cb84010 manual
// OFFSET: 0x004A8D10
void ScrnDestroy() {
    ILayerDestroy();
    IStockDestroy();
}

// VERIFY: 47b62661 auto
// OFFSET: 0x004A8D20
void ScrnInitialize(int32_t param) {
    ILayerInitialize();
    s_initParam = param;
    IStockInitialize();
}
