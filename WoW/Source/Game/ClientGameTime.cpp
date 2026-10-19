#include <cstdint>
#include "Engine/Source/Base/CDataStore.h"
#include "Engine/Source/ConsoleBase/IConsole.h"
#include "Engine/Source/Storm/H/storm.h"
#include "WoW/Source/Game/GameTime.h"
#include "WoW/Source/Net/Types.h"
#include "WoW/Source/WorldClient/DayNight.h"
#include "WoW/Source/WorldClient/DayNight__DNInfo.h"
#include "WoW/Source/WowSvcs/WowSvcsClient/ClientServices.h"

// STATIC: 0x00D37F80
static void (*s_timeCallbacks[4])(int32_t hour, int32_t minute);

// VERIFY: 811614fb auto
// OFFSET: 0x007E2730
static void UpdateTime() {
    DayNight::DNInfo* info = DayNight::GetInfo();
    info->m_timeOfDay = g_clientGameTime.GetHourAndMinutes();
    info->m_dayProgression = g_clientGameTime.GameTimeGetDayProgression();
    info->m_day = static_cast<float>(g_clientGameTime.GetDaysSinceEpoch());
    DayNight::Update(1, nullptr);

    for (uint32_t i = 0; i < sizeof(s_timeCallbacks) / sizeof(s_timeCallbacks[0]); i++) {
        if (s_timeCallbacks[i]) {
            s_timeCallbacks[i](g_clientGameTime.m_hour, g_clientGameTime.m_minute);
        }
    }

    char timeString[128];
    char text[256];
    SStrPrintf(text, sizeof(text), "Time set to %s", WowTime::WowGetTimeString(&g_clientGameTime, timeString, sizeof(timeString)));
    ConsoleWrite(text, DEFAULT_COLOR);
}

// VERIFY: 981ba009 auto
// OFFSET: 0x007E2840
static int32_t Packet_SMSG_GAMESPEED_SET(void* param, NETMESSAGE msgId, uint32_t time, CDataStore* msg) {
    float minutesPerSecond;
    msg->Get(minutesPerSecond);

    if (!msg->IsRead()) {
        ConsoleWriteA("Malformed message recieved: Id = %d, Len = %d, Read = %d\n", DEFAULT_COLOR, msgId, msg->m_size, msg->m_read);
        return 0;
    }

    char text[256];
    SStrPrintf(text, sizeof(text), "Gamespeed set from %.03f to %.03f", g_clientGameTime.GameTimeSetMinutesPerSecond(minutesPerSecond), minutesPerSecond);
    ConsoleWrite(text, DEFAULT_COLOR);
    return 1;
}

// VERIFY: 2a90f752 auto
// OFFSET: 0x007E28E0
static int32_t Packet_SMSG_LOGIN_SETTIMESPEED(void* param, NETMESSAGE msgId, uint32_t time, CDataStore* msg) {
    uint32_t encodedTime;
    float minutesPerSecond;
    uint32_t tzHours;
    msg->Get(encodedTime);
    msg->Get(minutesPerSecond);
    msg->Get(tzHours);

    if (!msg->IsRead()) {
        ConsoleWriteA("Malformed message recieved: Id = %d, Len = %d, Read = %d\n", DEFAULT_COLOR, msgId, msg->m_size, msg->m_read);
        return 0;
    }

    WowTime gameTime;
    WowTime::WowDecodeTime(encodedTime, &gameTime);
    gameTime.m_tzHours = tzHours;
    g_clientGameTime.GameTimeSetTime(gameTime, true);
    UpdateTime();

    char text[256];
    SStrPrintf(text, sizeof(text), "Gamespeed set from %.03f to %.03f", g_clientGameTime.GameTimeSetMinutesPerSecond(minutesPerSecond), minutesPerSecond);
    ConsoleWrite(text, DEFAULT_COLOR);
    return 1;
}

// VERIFY: 097dc233 auto
// OFFSET: 0x007E29C0
static int32_t Packet_SMSG_GAMETIME_UPDATE(void* param, NETMESSAGE msgId, uint32_t time, CDataStore* msg) {
    uint32_t encodedTime;
    uint32_t tzHours = 0;
    msg->Get(encodedTime);
    msg->Get(tzHours);

    if (!msg->IsRead()) {
        ConsoleWriteA("Malformed message recieved: Id = %d, Len = %d, Read = %d\n", DEFAULT_COLOR, msgId, msg->m_size, msg->m_read);
        return 0;
    }

    WowTime gameTime;
    WowTime::WowDecodeTime(encodedTime, &gameTime);
    gameTime.m_tzHours = tzHours;
    g_clientGameTime.GameTimeSync(gameTime, false);
    return 1;
}

// VERIFY: 7ffa0808 auto
// OFFSET: 0x007E2A50
static int32_t Packet_SMSG_SERVERTIME(void* param, NETMESSAGE msgId, uint32_t time, CDataStore* msg) {
    uint32_t encodedTime;
    uint32_t lastTick;
    msg->Get(encodedTime);
    msg->Get(lastTick);

    if (!msg->IsRead()) {
        ConsoleWriteA("Malformed message recieved: Id = %d, Len = %d, Read = %d\n", DEFAULT_COLOR, msgId, msg->m_size, msg->m_read);
        return 0;
    }

    WowTime serverTime;
    WowTime::WowDecodeTime(encodedTime, &serverTime);
    serverTime.m_tzHours = 0;

    char timeString[128];
    char text[256];
    SStrPrintf(text, sizeof(text), "Server game time is %s", WowTime::WowGetTimeString(&serverTime, timeString, sizeof(timeString)));
    ConsoleWrite(text, DEFAULT_COLOR);
    SStrPrintf(text, sizeof(text), "Server last tick is 0x%08x %s", lastTick, static_cast<int32_t>(lastTick) < 0 ? "(High bit set)" : "");
    ConsoleWrite(text, DEFAULT_COLOR);
    return 1;
}

// VERIFY: 4c53ebdb auto
// OFFSET: 0x007E2B40
static int32_t Packet_SMSG_GAMETIME_SET(void* param, NETMESSAGE msgId, uint32_t time, CDataStore* msg) {
    uint32_t encodedTime;
    uint32_t tzHours;
    msg->Get(encodedTime);
    msg->Get(tzHours);

    if (!msg->IsRead()) {
        ConsoleWriteA("Malformed message recieved: Id = %d, Len = %d, Read = %d\n", DEFAULT_COLOR, msgId, msg->m_size, msg->m_read);
        return 0;
    }

    WowTime gameTime;
    WowTime::WowDecodeTime(encodedTime, &gameTime);
    gameTime.m_tzHours = tzHours;
    g_clientGameTime.GameTimeSetTime(gameTime, true);
    UpdateTime();
    return 1;
}

// VERIFY: e0187f7d auto
// OFFSET: 0x007E2BD0
void ClientInitializeGameTime() {
    ClientServices::SetMessageHandler(SMSG_GAME_SPEED_SET, Packet_SMSG_GAMESPEED_SET, nullptr);
    ClientServices::SetMessageHandler(SMSG_LOGIN_SET_TIME_SPEED, Packet_SMSG_LOGIN_SETTIMESPEED, nullptr);
    ClientServices::SetMessageHandler(SMSG_GAME_TIME_UPDATE, Packet_SMSG_GAMETIME_UPDATE, nullptr);
    ClientServices::SetMessageHandler(SMSG_SERVERTIME, Packet_SMSG_SERVERTIME, nullptr);
    ClientServices::SetMessageHandler(SMSG_GAME_TIME_SET, Packet_SMSG_GAMETIME_SET, nullptr);

    for (uint32_t i = 0; i < sizeof(s_timeCallbacks) / sizeof(s_timeCallbacks[0]); i++) {
        s_timeCallbacks[i] = nullptr;
    }
}

// VERIFY: 3f0c07d8 auto
// OFFSET: 0x007E27F0
int32_t ClientGameTimeTickHandler(const void* data, void* param) {
    g_clientGameTime.GameTimeUpdate(*static_cast<const float*>(data));
    return 1;
}

// VERIFY: c94f4e8a manual
// OFFSET: 0x007E2810
void ClientDestroyGameTime() {
    ClientServices::ClearMessageHandler(SMSG_GAME_SPEED_SET);
    ClientServices::ClearMessageHandler(SMSG_LOGIN_SET_TIME_SPEED);
    ClientServices::ClearMessageHandler(SMSG_GAME_TIME_UPDATE);
    ClientServices::ClearMessageHandler(SMSG_SERVERTIME);
    ClientServices::ClearMessageHandler(SMSG_GAME_TIME_SET);
    g_clientGameTime.m_timeStamps.Clear();
}
