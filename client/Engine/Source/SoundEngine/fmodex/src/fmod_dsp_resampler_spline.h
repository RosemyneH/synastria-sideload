#ifndef _FMOD_DSP_RESAMPLER_SPLINE_H
#define _FMOD_DSP_RESAMPLER_SPLINE_H

#include "fmod_settings.h"

#include "fmod.h"
#include "fmod_types.h"

#ifdef __cplusplus
extern "C"
{
#endif

    void FMOD_Resampler_Spline(float *out, int outlength, void *src, FMOD_SOUND_FORMAT srcformat, FMOD_UINT64P *position, FMOD_SINT64P *speed, int channels);

#ifdef __cplusplus
}
#endif

#endif  

