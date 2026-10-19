#include "Engine/Source/Base/Coordinate.h"
#include "Engine/Source/Event/CMouseEvent.h"

// VERIFY: ed0a27f8 manual
// OFFSET: 0x0055DCE0
CMouseEvent::CMouseEvent() {
    this->m_data.m_mousePosition.x = 0.5f;
    this->m_data.m_mousePosition.y = 0.5f;
}

// VERIFY: c018945e auto
// OFFSET: 0x00480DE0
CMouseEvent& CMouseEvent::operator=(const EVENT_DATA_MOUSE& data) {
    this->m_data.m_mode = data.m_mode;
    this->m_data.m_button = data.m_button;
    this->m_data.m_buttonState = data.m_buttonState;
    this->m_data.m_metaKeyState = data.m_metaKeyState;
    this->m_data.m_flags = data.m_flags;
    this->m_data.m_eventTime = data.m_time;
    this->m_data.m_wheelAmount = data.m_wheelDistance;
    NDCToDDC(data.m_x, data.m_y, &this->m_data.m_mousePosition.x, &this->m_data.m_mousePosition.y);

    return *this;
}
