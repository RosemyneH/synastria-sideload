#ifndef ENGINE_SOURCE_BLIZZARDCORE_SOURCE_PACKAGES_STREAMING_TRACKER_H
#define ENGINE_SOURCE_BLIZZARDCORE_SOURCE_PACKAGES_STREAMING_TRACKER_H

#include <cstdint>
#include <string>
#include "Engine/Source/BlizzardCore/Source/System/Lock/Lock.h"
#include "Engine/Source/BlizzardCore/Source/System/URL/URL.h"

namespace Tracker {

class TrackerHttpDataDownloadCallBack : public Blizzard::URL::DownloadCallback {
    public:
    /* 0004 */ std::string m_data;
    /* 0020 */ uint32_t m_timeout;
    /* 0024 */ Blizzard::Lock::Event m_event;
    /* 0028 */ bool m_shouldBlock;

    ~TrackerHttpDataDownloadCallBack() override;
    int32_t GetTimeout() override;
    bool ShouldBlock() override;
    bool ReceivedData(const void* data, int32_t size) override;
    void Completed(Blizzard::URL::DownloadCallback::Status status) override;
    virtual void SetTimeOutValue(uint32_t timeout);

    TrackerHttpDataDownloadCallBack();
};

extern const char* trackerAddress;

std::string ByteToHex(uint8_t byte);
std::string BytesToHex(const uint8_t* bytes, int32_t count);
char* GetKey();
char* GetPeerID();
const char* GetTrackerAddress(const char* locale);
std::string MakeTrackerURL(const char* address, const char* infoHash, int32_t event, int64_t downloaded, const char* stats);
std::string Quote(const char* str, uint32_t length);
void SendToTracker(const char* infoHash, int32_t event, int64_t downloaded, const char* stats);
void SetBuildnum(int32_t buildnum);
void SetLocale(const char* locale);

}

void StartupReportFrameCallback();
void StartupReportSendTelemetry(bool start, bool unique);
void StartupReportSetSpeedTest(int32_t speedTest);

#endif
