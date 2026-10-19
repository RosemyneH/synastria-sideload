#ifndef ENGINE_SOURCE_COMSAT_COMSATALLOCATOR_H
#define ENGINE_SOURCE_COMSAT_COMSATALLOCATOR_H

#include <cstddef>
#include <cstdint>
#include "Engine/Source/ComSat/ComSatInterface.h"

namespace ComSat {

template <class T>
void Delete(Allocator& allocator, T* object);

}

// INLINED
template <class T>
void ComSat::Delete(ComSat::Allocator& allocator, T* object) {
    object->~T();
    allocator.Free(object);
}

#endif
