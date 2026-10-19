#ifndef _FMOD_DSP_RESAMPLER_NOINTERP_H
#define _FMOD_DSP_RESAMPLER_NOINTERP_H

#include "fmod_settings.h"

#include "fmod.h"
#include "fmod_types.h"

#ifdef __cplusplus
extern "C"
{
#endif

    void FMOD_Resampler_NoInterp(float *out, int outlength, void *src, FMOD_SOUND_FORMAT srcformat, FMOD_UINT64P *position, FMOD_SINT64P *speed, int channels);

#ifdef __cplusplus
}
#endif

#endif  

