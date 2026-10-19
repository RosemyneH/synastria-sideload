#include "URL.h"
#include <cstdint>

// VERIFY: b4a4f159 auto
// OFFSET: 0x00426DF0
Blizzard::URL::DownloadCallback::~DownloadCallback() {
}

// VERIFY: 0376006c auto
// OFFSET: 0x008E5250
const char* Blizzard::URL::DownloadCallback::WantIfModifiedSince() {
    return nullptr;
}

// VERIFY: 9ebe0d4f auto
// OFFSET: 0x008A1420
bool Blizzard::URL::DownloadCallback::WantData() {
    return true;
}

// VERIFY: deafea3c auto
// OFFSET: 0x00632050
void Blizzard::URL::DownloadCallback::GetLastModified(const char* lastModified) {
}

// VERIFY: 21209c1c auto
// OFFSET: 0x00653A10
void Blizzard::URL::DownloadCallback::GetContentLength(int64_t length) {
}

// VERIFY: 1d4642d1 auto
// OFFSET: 0x005EEB70
void Blizzard::URL::DownloadCallback::ConnectedOK() {
}

// VERIFY: abe175a0 auto
// OFFSET: 0x00632050
void Blizzard::URL::DownloadCallback::GetHTTPStatusCode(int32_t code) {
}

// VERIFY: acf356a7 auto
// OFFSET: 0x00426DA0
int32_t Blizzard::URL::DownloadCallback::GetTimeout() {
    return 60000;
}

// VERIFY: 206d184f auto
// OFFSET: 0x00426DB0
Blizzard::URL::DownloadCallback::ByteRange Blizzard::URL::DownloadCallback::GetByteRange() {
    ByteRange range;
    range.m_start = -1;
    range.m_end = -1;
    return range;
}

// VERIFY: 845a2a0f auto
// OFFSET: 0x00427A90
bool Blizzard::URL::DownloadCallback::Stop() {
    return false;
}

// VERIFY: 4f923f93 auto
// OFFSET: 0x00427A90
bool Blizzard::URL::DownloadCallback::ShouldBlock() {
    return false;
}

// VERIFY: d002f513 auto
// OFFSET: 0x008E5250
const char* Blizzard::URL::DownloadCallback::GetPostData() {
    return nullptr;
}

// VERIFY: 320609ee auto
// OFFSET: 0x008E5250
const char* Blizzard::URL::DownloadCallback::GetPostContentType() {
    return nullptr;
}

// VERIFY: 514fdb0b auto
// OFFSET: 0x00426DD0
Blizzard::URL::DownloadCallback::Cookie Blizzard::URL::DownloadCallback::GetCookie() {
    Cookie cookie;
    cookie.m_name = nullptr;
    cookie.m_value = nullptr;
    return cookie;
}
