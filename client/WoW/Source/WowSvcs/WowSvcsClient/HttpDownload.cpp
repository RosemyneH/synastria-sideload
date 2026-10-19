#include "HttpDownload.h"
#include <cstdint>
#include "Engine/Source/BlizzardCore/Source/System/Memory/MemoryStorm.h"
#include "Engine/Source/BlizzardCore/Source/System/String/String.h"
#include "Engine/Source/BlizzardCore/Source/System/Thread/Thread.h"

// INLINED
DownloadFileCallback::DownloadFileCallback() {
    this->m_stop = false;
}

// VERIFY: 379d687c manual
// OFFSET: 0x008A1330
void DownloadFileCallback::Completed(Blizzard::URL::DownloadCallback::Status status) {
    if (this->m_state) {
        this->m_state->m_done = true;
    }

    delete this;
}

// VERIFY: 18354f27 manual
// OFFSET: 0x008A1350
void DownloadFileCallback::GetContentLength(int64_t length) {
    if (this->m_state) {
        this->m_state->m_size = length;
    }
}

// VERIFY: cc9642a9 manual
// OFFSET: 0x008A1370
bool HttpDownload::Done() {
    return !this->m_requested || this->m_state->m_done;
}

// VERIFY: 5c3f7d88 manual
// OFFSET: 0x008A1390
bool HttpDownload::Cancel() {
    if (!this->m_requested || this->m_state->m_done) {
        return true;
    }

    this->m_callback->m_stop = true;
    return false;
}

// VERIFY: 7e0e4c37 manual
// OFFSET: 0x008A13B0
uint8_t* HttpDownload::GetBytes() {
    return this->m_state->m_data;
}

// VERIFY: c62441e5 manual
// OFFSET: 0x008A13C0
int64_t HttpDownload::Size() {
    if (this->m_state->m_done) {
        this->m_state->m_size = this->m_state->m_received;
    }

    if (this->m_state->m_size < this->m_state->m_received) {
        this->m_state->m_size = 2 * this->m_state->m_received;
    }

    return this->m_state->m_size;
}

// VERIFY: 66e5722c manual
// OFFSET: 0x008A1410
int64_t HttpDownload::Progress() {
    return this->m_state->m_received;
}

// VERIFY: 5bc19210 auto
// OFFSET: 0x008A1420
bool DownloadFileCallback::WantData() {
    return true;
}

// VERIFY: 78dc384f auto
// OFFSET: 0x008A1430
bool DownloadFileCallback::Stop() {
    return this->m_stop;
}

// VERIFY: 3075fdd3 auto
// OFFSET: 0x00427A90
bool DownloadFileCallback::ShouldBlock() {
    return false;
}

// VERIFY: 7ed85eec manual
// OFFSET: 0x008A1440
bool DownloadFileCallback::ReceivedData(const void* data, int32_t size) {
    if (!this->m_state) {
        return true;
    }

    this->m_state->m_receiving = true;

    if (!this->m_state->m_data) {
        this->m_state->m_capacity = static_cast<int32_t>(this->m_state->m_size);

        if (this->m_state->m_capacity < 0) {
            this->m_state->m_capacity = size;
        }

        this->m_state->m_data = static_cast<uint8_t*>(Blizzard::Memory::Allocate(this->m_state->m_capacity));
    }

    if (this->m_state->m_received + size > this->m_state->m_capacity) {
        this->m_state->m_capacity *= 2;

        if (this->m_state->m_capacity < this->m_state->m_received + size) {
            this->m_state->m_capacity = this->m_state->m_received + size;
        }

        uint8_t* grown = static_cast<uint8_t*>(Blizzard::Memory::Allocate(this->m_state->m_capacity));
        Blizzard::String::MemCopy(grown, this->m_state->m_data, this->m_state->m_received);
        Blizzard::Memory::Free(this->m_state->m_data);
        this->m_state->m_data = grown;
    }

    Blizzard::String::MemCopy(this->m_state->m_data + this->m_state->m_received, data, size);
    this->m_state->m_received += size;
    this->m_state->m_receiving = false;

    return true;
}

// VERIFY: 5caeaac4 manual
// OFFSET: 0x008A1530
const char* HttpDownload::TargetName() {
    return this->m_state->m_targetName.c_str();
}

// VERIFY: 46de26a1 manual
// OFFSET: 0x008A1550
HttpDownload::~HttpDownload() {
    if (!this->m_state->m_done) {
        while (this->m_state->m_receiving) {
            Blizzard::Thread::Sleep(0);
        }

        this->m_callback->m_state = nullptr;
    }

    Blizzard::Memory::Free(this->m_state->m_data);
    delete this->m_state;
}

// VERIFY: 0a70c43e manual
// OFFSET: 0x008A15E0
HttpDownload::HttpDownload(const char* url, const char* targetName) {
    this->m_state = new HttpDownloadState;
    this->m_state->m_receiving = false;
    this->m_state->m_done = false;
    this->m_state->m_size = -1;
    this->m_state->m_data = nullptr;
    this->m_state->m_received = 0;

    this->m_callback = new DownloadFileCallback;
    this->m_callback->m_state = this->m_state;
    this->m_state->m_url = url;

    if (targetName) {
        this->m_state->m_targetName = targetName;
    }

    this->m_requested = Blizzard::URL::DownloadURL(url, this->m_callback);
}
