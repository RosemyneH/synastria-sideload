#ifndef ENGINE_SOURCE_GX_CGXDEVICEOPENGL_CGXVBOBROKER1_H
#define ENGINE_SOURCE_GX_CGXDEVICEOPENGL_CGXVBOBROKER1_H

#include <cstddef>
#include <cstdint>
#include "Engine/Source/Gx/CGxDeviceOpenGL/GlExt.h"

class CGxDeviceOpenGl;

class CGxVboBroker {
    public:
    virtual ~CGxVboBroker();
    virtual void SelectVbo(uint32_t size, GLuint& vbo, uint32_t& offset) = 0;
    virtual void GetLastSelectedVbo(GLuint& vbo, uint32_t& offset) = 0;
    virtual void UnlockVbo(uint32_t offset, uint32_t size) = 0;
    virtual void Reset() = 0;
};

class CGxVboBroker1 : public CGxVboBroker {
    public:
    /* 0004 */ CGxDeviceOpenGl* m_device;
    /* 0008 */ GLenum m_target;
    /* 000C */ uint32_t m_maxBits;
    /* 0010 */ uint32_t m_slotCount [25];
    /* 0074 */ uint32_t m_slotCursor [25];
    /* 00D8 */ uint32_t m_unkD8 [25];
    /* 013C */ GLuint m_vbos [25][1200];
    /* 1D5FC */ GLuint m_lastVbo;
    /* 1D600 */ uint32_t m_selectCount;

    CGxVboBroker1(GLenum target, uint32_t maxBits, CGxDeviceOpenGl* device);
    virtual ~CGxVboBroker1();
    virtual void SelectVbo(uint32_t size, GLuint& vbo, uint32_t& offset);
    virtual void GetLastSelectedVbo(GLuint& vbo, uint32_t& offset);
    virtual void UnlockVbo(uint32_t offset, uint32_t size);
    virtual void Reset();
};

#endif
