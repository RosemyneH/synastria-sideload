#include <cmath>
#include "Engine/Source/Tempest/C3Ray.h"

// VERIFY: a5d6e75f manual
// OFFSET: 0x00985200
C3Ray C3Ray::FromStartEnd(const C3Vector& start, const C3Vector& end, bool normalize) {
    float x = end.x - start.x;
    float y = end.y - start.y;
    float z = end.z - start.z;

    if (normalize) {
        float scale = 1.0f / sqrt(z * z + y * y + x * x);
        x = x * scale;
        y = y * scale;
        z = scale * z;
    }

    C3Ray result;
    result.origin = start;
    result.dir.x = x;
    result.dir.y = y;
    result.dir.z = z;
    return result;
}
