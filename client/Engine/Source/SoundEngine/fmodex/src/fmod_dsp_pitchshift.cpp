/****************************************************************************
*
* NAME: smbPitchShift.cpp
* VERSION: 1.1
* HOME URL: http://www.dspdimension.com
* KNOWN BUGS: none
*
* SYNOPSIS: Routine for doing pitch shifting while maintaining
* duration using the Short Time Fourier Transform.
*
* DESCRIPTION: The routine takes a pitchShift factor value which is between 0.5
* (one octave down) and 2. (one octave up). A value of exactly 1 does not change
* the pitch. numSampsToProcess tells the routine how many samples in indata[0...
* numSampsToProcess-1] should be pitch shifted and moved to outdata[0 ...
* numSampsToProcess-1]. The two buffers can be identical (ie. it can process the
* data in-place). fftFrameSize defines the FFT frame size used for the
* processing. Typical values are 1024, 2048 and 4096. It may be any value <=
* MAX_FFT_FRAME_LENGTH but it MUST be a power of 2. osamp is the STFT
* oversampling factor which also determines the overlap between adjacent STFT
* frames. It should at least be 4 for moderate scaling ratios. A value of 32 is
* recommended for best quality. sampleRate takes the sample rate for the signal 
* in unit Hz, ie. 44100 for 44.1 kHz audio. The data passed to the routine in 
* indata[] should be in the range [-1.0, 1.0), which is also the output range 
* for the data, make sure you scale the data accordingly (for 16bit signed integers
* you would have to divide (and multiply) by 32768). 
*
* COPYRIGHT 1999-2003 Stephan M. Bernsee <smb@dspdimension.com>
*
* 						The Wide Open License (WOL)
*
* Permission to use, copy, modify, distribute and sell this software and its
* documentation for any purpose is hereby granted without fee, provided that
* the above copyright notice and this license appear in all source copies. 
* THIS SOFTWARE IS PROVIDED "AS IS" WITHOUT EXPRESS OR IMPLIED WARRANTY OF
* ANY KIND. See http://www.dspguru.com/wol.htm for more information.
*
*****************************************************************************/ 

#include "fmod_settings.h"

#ifdef FMOD_SUPPORT_PITCHSHIFT

#include "fmod.h"
#include "fmod_3d.h"
#include "fmod_dspi.h"
#include "fmod_dsp_pitchshift.h"
#include "fmod_systemi.h"

#include <stdio.h>

namespace FMOD
{

FMOD_DSP_DESCRIPTION_EX dsppitchshift;

#ifdef PLUGIN_EXPORTS

#ifdef __cplusplus
extern "C" {
#endif

    /*
        FMODGetDSPDescription is mandantory for every fmod plugin.  This is the symbol the registerplugin function searches for.
        Must be declared with F_API to make it export as stdcall.
    */
    F_DECLSPEC F_DLLEXPORT FMOD_DSP_DESCRIPTION_EX * F_API FMODGetDSPDescriptionEx()
    {
        return DSPPitchShift::getDescriptionEx();
    }

#ifdef __cplusplus
}
#endif

#endif  /* PLUGIN_EXPORTS */


FMOD_DSP_PARAMETERDESC dsppitchshift_param[4] =
{
    {    0.5f,    2.0f,    1.0f, "Pitch",        "x",        "Pitch value.  0.5 to 2.0.  Default = 1.0. 0.5 = one octave down, 2.0 = one octave up.  1.0 does not change the pitch." },
    {  256.0f, 4096.0f, 1024.0f, "FFT size",     "",         "FFT window size.  256, 512, 1024, 2048, 4096.  Default = 1024.  Increase this to reduce 'smearing'.  This effect is a warbling sound similar to when an mp3 is encoded at very low bitrates." },
    {    1.0f,   32.0f,    4.0f, "Overlap",      "",         "Window overlap.  1 to 32.  Default = 4.  Increase this to reduce 'tremolo' effect.  Increasing it by a factor of 2 doubles the CPU usage." },
    {    0.0f,   16.0f,    0.0f, "Max channels", "channels", "Maximum channels supported.  0 to 16.  0 = same as fmod's default output polyphony, 1 = mono, 2 = stereo etc.  See remarks for more.  Default = 0.  It is suggested to leave at 0!" }
};


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Macintosh, XBox, PlayStation 2, GameCube

	[SEE_ALSO]
]
*/
// VERIFY: 8b7e2957 auto
// OFFSET: 0x008EDE90
FMOD_DSP_DESCRIPTION_EX *DSPPitchShift::getDescriptionEx()
{
    FMOD_memset(&dsppitchshift, 0, sizeof(FMOD_DSP_DESCRIPTION_EX));

    FMOD_strcpy(dsppitchshift.name, "FMOD Pitch Shifter");
    dsppitchshift.version       = 0x00010100;
    dsppitchshift.create        = DSPPitchShift::createCallback;
    dsppitchshift.release       = DSPPitchShift::releaseCallback;
    dsppitchshift.reset         = DSPPitchShift::resetCallback;
    dsppitchshift.read          = DSPPitchShift::readCallback;

    dsppitchshift.numparameters = sizeof(dsppitchshift_param) / sizeof(dsppitchshift_param[0]);
    dsppitchshift.paramdesc     = dsppitchshift_param;
    dsppitchshift.setparameter  = DSPPitchShift::setParameterCallback;
    dsppitchshift.getparameter  = DSPPitchShift::getParameterCallback;

    dsppitchshift.mType         = FMOD_DSP_TYPE_PITCHSHIFT;
    dsppitchshift.mCategory     = FMOD_DSP_CATEGORY_FILTER;
    dsppitchshift.mSize         = sizeof(DSPPitchShift);

    return &dsppitchshift;
}


float           gFFTworksp  [MAX_FRAME_LENGTH * 2];
float           gFFTtable   [MAX_FRAME_LENGTH / 2];
int             gFFTbitrev  [2 + (1<<MAX_FRAME_LENGTH_HALF_BITS)];
float           gSynFreq    [MAX_FRAME_LENGTH];
float           gSynMagn    [MAX_FRAME_LENGTH];
float           gAnaFreq    [MAX_FRAME_LENGTH];
float           gAnaMagn    [MAX_FRAME_LENGTH];


void DSPPitchShiftSMB::initFft(int fftSize)
{
    int j, nwh;
    float delta, x, y;
    
    gFFTbitrev[0] = fftSize >> 1;
    gFFTbitrev[1] = 1;

    nwh = fftSize >> 2;
#ifdef DSP_PITCHSHIFT_USECOSTAB
    delta = 1.0f / (8.0f * nwh);
#else
    delta = atan(1.0f) / nwh;
#endif
    gFFTtable[0] = 1;
    gFFTtable[1] = 0;
#ifdef DSP_PITCHSHIFT_USECOSTAB
    gFFTtable[nwh] = cosine(1.0f / 8.0f); //cos(pi/4)
#else
    gFFTtable[nwh] = cos(delta * nwh);
#endif
    gFFTtable[nwh + 1] = gFFTtable[nwh];
    for (j = 2; j < nwh; j += 2) {
#ifdef DSP_PITCHSHIFT_USECOSTAB
        x = cosine(delta * j);
        y = sine(delta * j);
#else
        x = cos(delta * j);
        y = sin(delta * j);
#endif
        gFFTtable[j] = x;
        gFFTtable[j + 1] = y;
        gFFTtable[gFFTbitrev[0] - j] = y;
        gFFTtable[gFFTbitrev[0] - j + 1] = x;
    }
    bitrv2(gFFTtable, gFFTbitrev[0]);
}

void DSPPitchShiftSMB::bitrv2(float *data, int count)
{
    int j, j1, k, k1, m, m2;
    float xr, xi, yr, yi;
    int *ip = gFFTbitrev + 2;

    ip[0] = 0;
    m = 1;
    while ((m << 3) < count) {
        count >>= 1;
        for (j = 0; j < m; j++) {
            ip[m + j] = ip[j] + count;
        }
        m <<= 1;
    }
    m2 = 2 * m;
    if ((m << 3) == count) {
        for (k = 0; k < m; k++) {
            for (j = 0; j < k; j++) {
                j1 = 2 * j + ip[k];
                k1 = 2 * k + ip[j];
                xr = data[j1];
                xi = data[j1 + 1];
                yr = data[k1];
                yi = data[k1 + 1];
                data[j1] = yr;
                data[j1 + 1] = yi;
                data[k1] = xr;
                data[k1 + 1] = xi;
                j1 += m2;
                k1 += 2 * m2;
                xr = data[j1];
                xi = data[j1 + 1];
                yr = data[k1];
                yi = data[k1 + 1];
                data[j1] = yr;
                data[j1 + 1] = yi;
                data[k1] = xr;
                data[k1 + 1] = xi;
                j1 += m2;
                k1 -= m2;
                xr = data[j1];
                xi = data[j1 + 1];
                yr = data[k1];
                yi = data[k1 + 1];
                data[j1] = yr;
                data[j1 + 1] = yi;
                data[k1] = xr;
                data[k1 + 1] = xi;
                j1 += m2;
                k1 += 2 * m2;
                xr = data[j1];
                xi = data[j1 + 1];
                yr = data[k1];
                yi = data[k1 + 1];
                data[j1] = yr;
                data[j1 + 1] = yi;
                data[k1] = xr;
                data[k1 + 1] = xi;
            }
            j1 = 2 * k + m2 + ip[k];
            k1 = j1 + m2;
            xr = data[j1];
            xi = data[j1 + 1];
            yr = data[k1];
            yi = data[k1 + 1];
            data[j1] = yr;
            data[j1 + 1] = yi;
            data[k1] = xr;
            data[k1 + 1] = xi;
        }
    } else {
        for (k = 1; k < m; k++) {
            for (j = 0; j < k; j++) {
                j1 = 2 * j + ip[k];
                k1 = 2 * k + ip[j];
                xr = data[j1];
                xi = data[j1 + 1];
                yr = data[k1];
                yi = data[k1 + 1];
                data[j1] = yr;
                data[j1 + 1] = yi;
                data[k1] = xr;
                data[k1 + 1] = xi;
                j1 += m2;
                k1 += m2;
                xr = data[j1];
                xi = data[j1 + 1];
                yr = data[k1];
                yi = data[k1 + 1];
                data[j1] = yr;
                data[j1 + 1] = yi;
                data[k1] = xr;
                data[k1 + 1] = xi;
            }
        }
    }
}

void DSPPitchShiftSMB::bitrv2conj(float *data, int count)
{
    int j, j1, k, k1, m, m2;
    float xr, xi, yr, yi;
    int *ip = gFFTbitrev + 2;
    
    ip[0] = 0;
    m = 1;
    while ((m << 3) < count) {
        count >>= 1;
        for (j = 0; j < m; j++) {
            ip[m + j] = ip[j] + count;
        }
        m <<= 1;
    }
    m2 = 2 * m;
    if ((m << 3) == count) {
        for (k = 0; k < m; k++) {
            for (j = 0; j < k; j++) {
                j1 = 2 * j + ip[k];
                k1 = 2 * k + ip[j];
                xr = data[j1];
                xi = -data[j1 + 1];
                yr = data[k1];
                yi = -data[k1 + 1];
                data[j1] = yr;
                data[j1 + 1] = yi;
                data[k1] = xr;
                data[k1 + 1] = xi;
                j1 += m2;
                k1 += 2 * m2;
                xr = data[j1];
                xi = -data[j1 + 1];
                yr = data[k1];
                yi = -data[k1 + 1];
                data[j1] = yr;
                data[j1 + 1] = yi;
                data[k1] = xr;
                data[k1 + 1] = xi;
                j1 += m2;
                k1 -= m2;
                xr = data[j1];
                xi = -data[j1 + 1];
                yr = data[k1];
                yi = -data[k1 + 1];
                data[j1] = yr;
                data[j1 + 1] = yi;
                data[k1] = xr;
                data[k1 + 1] = xi;
                j1 += m2;
                k1 += 2 * m2;
                xr = data[j1];
                xi = -data[j1 + 1];
                yr = data[k1];
                yi = -data[k1 + 1];
                data[j1] = yr;
                data[j1 + 1] = yi;
                data[k1] = xr;
                data[k1 + 1] = xi;
            }
            k1 = 2 * k + ip[k];
            data[k1 + 1] = -data[k1 + 1];
            j1 = k1 + m2;
            k1 = j1 + m2;
            xr = data[j1];
            xi = -data[j1 + 1];
            yr = data[k1];
            yi = -data[k1 + 1];
            data[j1] = yr;
            data[j1 + 1] = yi;
            data[k1] = xr;
            data[k1 + 1] = xi;
            k1 += m2;
            data[k1 + 1] = -data[k1 + 1];
        }
    } else {
        data[1] = -data[1];
        data[m2 + 1] = -data[m2 + 1];
        for (k = 1; k < m; k++) {
            for (j = 0; j < k; j++) {
                j1 = 2 * j + ip[k];
                k1 = 2 * k + ip[j];
                xr = data[j1];
                xi = -data[j1 + 1];
                yr = data[k1];
                yi = -data[k1 + 1];
                data[j1] = yr;
                data[j1 + 1] = yi;
                data[k1] = xr;
                data[k1 + 1] = xi;
                j1 += m2;
                k1 += m2;
                xr = data[j1];
                xi = -data[j1 + 1];
                yr = data[k1];
                yi = -data[k1 + 1];
                data[j1] = yr;
                data[j1 + 1] = yi;
                data[k1] = xr;
                data[k1 + 1] = xi;
            }
            k1 = 2 * k + ip[k];
            data[k1 + 1] = -data[k1 + 1];
            data[k1 + m2 + 1] = -data[k1 + m2 + 1];
        }
    }
}

void DSPPitchShiftSMB::cft1st(float *fftBuffer)
{
    int j, k1, k2;
    float wk1r, wk1i, wk2r, wk2i, wk3r, wk3i;
    float x0r, x0i, x1r, x1i, x2r, x2i, x3r, x3i;
    
    x0r = fftBuffer[0] + fftBuffer[2];
    x0i = fftBuffer[1] + fftBuffer[3];
    x1r = fftBuffer[0] - fftBuffer[2];
    x1i = fftBuffer[1] - fftBuffer[3];
    x2r = fftBuffer[4] + fftBuffer[6];
    x2i = fftBuffer[5] + fftBuffer[7];
    x3r = fftBuffer[4] - fftBuffer[6];
    x3i = fftBuffer[5] - fftBuffer[7];
    fftBuffer[0] = x0r + x2r;
    fftBuffer[1] = x0i + x2i;
    fftBuffer[4] = x0r - x2r;
    fftBuffer[5] = x0i - x2i;
    fftBuffer[2] = x1r - x3i;
    fftBuffer[3] = x1i + x3r;
    fftBuffer[6] = x1r + x3i;
    fftBuffer[7] = x1i - x3r;
    wk1r = gFFTtable[2];
    x0r = fftBuffer[8] + fftBuffer[10];
    x0i = fftBuffer[9] + fftBuffer[11];
    x1r = fftBuffer[8] - fftBuffer[10];
    x1i = fftBuffer[9] - fftBuffer[11];
    x2r = fftBuffer[12] + fftBuffer[14];
    x2i = fftBuffer[13] + fftBuffer[15];
    x3r = fftBuffer[12] - fftBuffer[14];
    x3i = fftBuffer[13] - fftBuffer[15];
    fftBuffer[8] = x0r + x2r;
    fftBuffer[9] = x0i + x2i;
    fftBuffer[12] = x2i - x0i;
    fftBuffer[13] = x0r - x2r;
    x0r = x1r - x3i;
    x0i = x1i + x3r;
    fftBuffer[10] = wk1r * (x0r - x0i);
    fftBuffer[11] = wk1r * (x0r + x0i);
    x0r = x3i + x1r;
    x0i = x3r - x1i;
    fftBuffer[14] = wk1r * (x0i - x0r);
    fftBuffer[15] = wk1r * (x0i + x0r);
    k1 = 0;
    for (j = 16; j < (mFFTFrameSize << 1); j += 16) {
        k1 += 2;
        k2 = 2 * k1;
        wk2r = gFFTtable[k1];
        wk2i = gFFTtable[k1 + 1];
        wk1r = gFFTtable[k2];
        wk1i = gFFTtable[k2 + 1];
        wk3r = wk1r - 2 * wk2i * wk1i;
        wk3i = 2 * wk2i * wk1r - wk1i;
        x0r = fftBuffer[j] + fftBuffer[j + 2];
        x0i = fftBuffer[j + 1] + fftBuffer[j + 3];
        x1r = fftBuffer[j] - fftBuffer[j + 2];
        x1i = fftBuffer[j + 1] - fftBuffer[j + 3];
        x2r = fftBuffer[j + 4] + fftBuffer[j + 6];
        x2i = fftBuffer[j + 5] + fftBuffer[j + 7];
        x3r = fftBuffer[j + 4] - fftBuffer[j + 6];
        x3i = fftBuffer[j + 5] - fftBuffer[j + 7];
        fftBuffer[j] = x0r + x2r;
        fftBuffer[j + 1] = x0i + x2i;
        x0r -= x2r;
        x0i -= x2i;
        fftBuffer[j + 4] = wk2r * x0r - wk2i * x0i;
        fftBuffer[j + 5] = wk2r * x0i + wk2i * x0r;
        x0r = x1r - x3i;
        x0i = x1i + x3r;
        fftBuffer[j + 2] = wk1r * x0r - wk1i * x0i;
        fftBuffer[j + 3] = wk1r * x0i + wk1i * x0r;
        x0r = x1r + x3i;
        x0i = x1i - x3r;
        fftBuffer[j + 6] = wk3r * x0r - wk3i * x0i;
        fftBuffer[j + 7] = wk3r * x0i + wk3i * x0r;
        wk1r = gFFTtable[k2 + 2];
        wk1i = gFFTtable[k2 + 3];
        wk3r = wk1r - 2 * wk2r * wk1i;
        wk3i = 2 * wk2r * wk1r - wk1i;
        x0r = fftBuffer[j + 8] + fftBuffer[j + 10];
        x0i = fftBuffer[j + 9] + fftBuffer[j + 11];
        x1r = fftBuffer[j + 8] - fftBuffer[j + 10];
        x1i = fftBuffer[j + 9] - fftBuffer[j + 11];
        x2r = fftBuffer[j + 12] + fftBuffer[j + 14];
        x2i = fftBuffer[j + 13] + fftBuffer[j + 15];
        x3r = fftBuffer[j + 12] - fftBuffer[j + 14];
        x3i = fftBuffer[j + 13] - fftBuffer[j + 15];
        fftBuffer[j + 8] = x0r + x2r;
        fftBuffer[j + 9] = x0i + x2i;
        x0r -= x2r;
        x0i -= x2i;
        fftBuffer[j + 12] = -wk2i * x0r - wk2r * x0i;
        fftBuffer[j + 13] = -wk2i * x0i + wk2r * x0r;
        x0r = x1r - x3i;
        x0i = x1i + x3r;
        fftBuffer[j + 10] = wk1r * x0r - wk1i * x0i;
        fftBuffer[j + 11] = wk1r * x0i + wk1i * x0r;
        x0r = x1r + x3i;
        x0i = x1i - x3r;
        fftBuffer[j + 14] = wk3r * x0r - wk3i * x0i;
        fftBuffer[j + 15] = wk3r * x0i + wk3i * x0r;
    }
}


void DSPPitchShiftSMB::cftmdl(float *fftBuffer, int count)
{
    int j, j1, j2, j3, k, k1, k2, m, m2;
    float wk1r, wk1i, wk2r, wk2i, wk3r, wk3i;
    float x0r, x0i, x1r, x1i, x2r, x2i, x3r, x3i;
    
    m = count << 2;
    for (j = 0; j < count; j += 2) {
        j1 = j + count;
        j2 = j1 + count;
        j3 = j2 + count;
        x0r = fftBuffer[j] + fftBuffer[j1];
        x0i = fftBuffer[j + 1] + fftBuffer[j1 + 1];
        x1r = fftBuffer[j] - fftBuffer[j1];
        x1i = fftBuffer[j + 1] - fftBuffer[j1 + 1];
        x2r = fftBuffer[j2] + fftBuffer[j3];
        x2i = fftBuffer[j2 + 1] + fftBuffer[j3 + 1];
        x3r = fftBuffer[j2] - fftBuffer[j3];
        x3i = fftBuffer[j2 + 1] - fftBuffer[j3 + 1];
        fftBuffer[j] = x0r + x2r;
        fftBuffer[j + 1] = x0i + x2i;
        fftBuffer[j2] = x0r - x2r;
        fftBuffer[j2 + 1] = x0i - x2i;
        fftBuffer[j1] = x1r - x3i;
        fftBuffer[j1 + 1] = x1i + x3r;
        fftBuffer[j3] = x1r + x3i;
        fftBuffer[j3 + 1] = x1i - x3r;
    }
    wk1r = gFFTtable[2];
    for (j = m; j < count + m; j += 2) {
        j1 = j + count;
        j2 = j1 + count;
        j3 = j2 + count;
        x0r = fftBuffer[j] + fftBuffer[j1];
        x0i = fftBuffer[j + 1] + fftBuffer[j1 + 1];
        x1r = fftBuffer[j] - fftBuffer[j1];
        x1i = fftBuffer[j + 1] - fftBuffer[j1 + 1];
        x2r = fftBuffer[j2] + fftBuffer[j3];
        x2i = fftBuffer[j2 + 1] + fftBuffer[j3 + 1];
        x3r = fftBuffer[j2] - fftBuffer[j3];
        x3i = fftBuffer[j2 + 1] - fftBuffer[j3 + 1];
        fftBuffer[j] = x0r + x2r;
        fftBuffer[j + 1] = x0i + x2i;
        fftBuffer[j2] = x2i - x0i;
        fftBuffer[j2 + 1] = x0r - x2r;
        x0r = x1r - x3i;
        x0i = x1i + x3r;
        fftBuffer[j1] = wk1r * (x0r - x0i);
        fftBuffer[j1 + 1] = wk1r * (x0r + x0i);
        x0r = x3i + x1r;
        x0i = x3r - x1i;
        fftBuffer[j3] = wk1r * (x0i - x0r);
        fftBuffer[j3 + 1] = wk1r * (x0i + x0r);
    }
    k1 = 0;
    m2 = 2 * m;
    for (k = m2; k < (mFFTFrameSize << 1); k += m2) {
        k1 += 2;
        k2 = 2 * k1;
        wk2r = gFFTtable[k1];
        wk2i = gFFTtable[k1 + 1];
        wk1r = gFFTtable[k2];
        wk1i = gFFTtable[k2 + 1];
        wk3r = wk1r - 2 * wk2i * wk1i;
        wk3i = 2 * wk2i * wk1r - wk1i;
        for (j = k; j < count + k; j += 2) {
            j1 = j + count;
            j2 = j1 + count;
            j3 = j2 + count;
            x0r = fftBuffer[j] + fftBuffer[j1];
            x0i = fftBuffer[j + 1] + fftBuffer[j1 + 1];
            x1r = fftBuffer[j] - fftBuffer[j1];
            x1i = fftBuffer[j + 1] - fftBuffer[j1 + 1];
            x2r = fftBuffer[j2] + fftBuffer[j3];
            x2i = fftBuffer[j2 + 1] + fftBuffer[j3 + 1];
            x3r = fftBuffer[j2] - fftBuffer[j3];
            x3i = fftBuffer[j2 + 1] - fftBuffer[j3 + 1];
            fftBuffer[j] = x0r + x2r;
            fftBuffer[j + 1] = x0i + x2i;
            x0r -= x2r;
            x0i -= x2i;
            fftBuffer[j2] = wk2r * x0r - wk2i * x0i;
            fftBuffer[j2 + 1] = wk2r * x0i + wk2i * x0r;
            x0r = x1r - x3i;
            x0i = x1i + x3r;
            fftBuffer[j1] = wk1r * x0r - wk1i * x0i;
            fftBuffer[j1 + 1] = wk1r * x0i + wk1i * x0r;
            x0r = x1r + x3i;
            x0i = x1i - x3r;
            fftBuffer[j3] = wk3r * x0r - wk3i * x0i;
            fftBuffer[j3 + 1] = wk3r * x0i + wk3i * x0r;
        }
        wk1r = gFFTtable[k2 + 2];
        wk1i = gFFTtable[k2 + 3];
        wk3r = wk1r - 2 * wk2r * wk1i;
        wk3i = 2 * wk2r * wk1r - wk1i;
        for (j = k + m; j < count + (k + m); j += 2) {
            j1 = j + count;
            j2 = j1 + count;
            j3 = j2 + count;
            x0r = fftBuffer[j] + fftBuffer[j1];
            x0i = fftBuffer[j + 1] + fftBuffer[j1 + 1];
            x1r = fftBuffer[j] - fftBuffer[j1];
            x1i = fftBuffer[j + 1] - fftBuffer[j1 + 1];
            x2r = fftBuffer[j2] + fftBuffer[j3];
            x2i = fftBuffer[j2 + 1] + fftBuffer[j3 + 1];
            x3r = fftBuffer[j2] - fftBuffer[j3];
            x3i = fftBuffer[j2 + 1] - fftBuffer[j3 + 1];
            fftBuffer[j] = x0r + x2r;
            fftBuffer[j + 1] = x0i + x2i;
            x0r -= x2r;
            x0i -= x2i;
            fftBuffer[j2] = -wk2i * x0r - wk2r * x0i;
            fftBuffer[j2 + 1] = -wk2i * x0i + wk2r * x0r;
            x0r = x1r - x3i;
            x0i = x1i + x3r;
            fftBuffer[j1] = wk1r * x0r - wk1i * x0i;
            fftBuffer[j1 + 1] = wk1r * x0i + wk1i * x0r;
            x0r = x1r + x3i;
            x0i = x1i - x3r;
            fftBuffer[j3] = wk3r * x0r - wk3i * x0i;
            fftBuffer[j3 + 1] = wk3r * x0i + wk3i * x0r;
        }
    }
}

void DSPPitchShiftSMB::cftfsub(float *fftBuffer)
{
    int j, j1, j2, j3, l;
    float x0r, x0i, x1r, x1i, x2r, x2i, x3r, x3i;
    
    l = 2;
    if ((mFFTFrameSize << 1) > 8) {
        cft1st(fftBuffer);
        l = 8;
        while ((l << 2) < (mFFTFrameSize << 1)) {
            cftmdl(fftBuffer, l);
            l <<= 2;
        }
    }
    if ((l << 2) == (mFFTFrameSize << 1)) {
        for (j = 0; j < l; j += 2) {
            j1 = j + l;
            j2 = j1 + l;
            j3 = j2 + l;
            x0r = fftBuffer[j] + fftBuffer[j1];
            x0i = fftBuffer[j + 1] + fftBuffer[j1 + 1];
            x1r = fftBuffer[j] - fftBuffer[j1];
            x1i = fftBuffer[j + 1] - fftBuffer[j1 + 1];
            x2r = fftBuffer[j2] + fftBuffer[j3];
            x2i = fftBuffer[j2 + 1] + fftBuffer[j3 + 1];
            x3r = fftBuffer[j2] - fftBuffer[j3];
            x3i = fftBuffer[j2 + 1] - fftBuffer[j3 + 1];
            fftBuffer[j] = x0r + x2r;
            fftBuffer[j + 1] = x0i + x2i;
            fftBuffer[j2] = x0r - x2r;
            fftBuffer[j2 + 1] = x0i - x2i;
            fftBuffer[j1] = x1r - x3i;
            fftBuffer[j1 + 1] = x1i + x3r;
            fftBuffer[j3] = x1r + x3i;
            fftBuffer[j3 + 1] = x1i - x3r;
        }
    } else {
        for (j = 0; j < l; j += 2) {
            j1 = j + l;
            x0r = fftBuffer[j] - fftBuffer[j1];
            x0i = fftBuffer[j + 1] - fftBuffer[j1 + 1];
            fftBuffer[j] += fftBuffer[j1];
            fftBuffer[j + 1] += fftBuffer[j1 + 1];
            fftBuffer[j1] = x0r;
            fftBuffer[j1 + 1] = x0i;
        }
    }
}

void DSPPitchShiftSMB::cftbsub(float *fftBuffer)
{
    int j, j1, j2, j3, count;
    float x0r, x0i, x1r, x1i, x2r, x2i, x3r, x3i;
    
    count = 2;
    if ((mFFTFrameSize << 1) > 8) {
        cft1st(fftBuffer);
        count = 8;
        while ((count << 2) < (mFFTFrameSize << 1)) {
            cftmdl(fftBuffer, count);
            count <<= 2;
        }
    }
    if ((count << 2) == (mFFTFrameSize << 1)) {
        for (j = 0; j < count; j += 2) {
            j1 = j + count;
            j2 = j1 + count;
            j3 = j2 + count;
            x0r = fftBuffer[j] + fftBuffer[j1];
            x0i = -fftBuffer[j + 1] - fftBuffer[j1 + 1];
            x1r = fftBuffer[j] - fftBuffer[j1];
            x1i = -fftBuffer[j + 1] + fftBuffer[j1 + 1];
            x2r = fftBuffer[j2] + fftBuffer[j3];
            x2i = fftBuffer[j2 + 1] + fftBuffer[j3 + 1];
            x3r = fftBuffer[j2] - fftBuffer[j3];
            x3i = fftBuffer[j2 + 1] - fftBuffer[j3 + 1];
            fftBuffer[j] = x0r + x2r;
            fftBuffer[j + 1] = x0i - x2i;
            fftBuffer[j2] = x0r - x2r;
            fftBuffer[j2 + 1] = x0i + x2i;
            fftBuffer[j1] = x1r - x3i;
            fftBuffer[j1 + 1] = x1i - x3r;
            fftBuffer[j3] = x1r + x3i;
            fftBuffer[j3 + 1] = x1i + x3r;
        }
    } else {
        for (j = 0; j < count; j += 2) {
            j1 = j + count;
            x0r = fftBuffer[j] - fftBuffer[j1];
            x0i = -fftBuffer[j + 1] + fftBuffer[j1 + 1];
            fftBuffer[j] += fftBuffer[j1];
            fftBuffer[j + 1] = -fftBuffer[j + 1] - fftBuffer[j1 + 1];
            fftBuffer[j1] = x0r;
            fftBuffer[j1 + 1] = x0i;
        }
    }
}

void DSPPitchShiftSMB::fft(float *fftBuffer, int sign)
{
    if (sign >= 0) {
        bitrv2(fftBuffer, mFFTFrameSize << 1);
        cftfsub(fftBuffer);
    } else {
        bitrv2conj(fftBuffer, mFFTFrameSize << 1);
        cftbsub(fftBuffer);
    }
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Macintosh, XBox, PlayStation 2, GameCube

	[SEE_ALSO]
]
*/
// VERIFY: a15ad283 auto
// OFFSET: 0x008EC450
void DSPPitchShiftSMB::smbInit()
{   
	/* initialize our static arrays */
	FMOD_memset(mInFIFO,      0,     MAX_FRAME_LENGTH * sizeof(float));
	FMOD_memset(mOutFIFO,     0,     MAX_FRAME_LENGTH * sizeof(float));
	FMOD_memset(mLastPhase,   0, (4+(MAX_FRAME_LENGTH/2)) * sizeof(float));
	FMOD_memset(mSumPhase,    0, (4+(MAX_FRAME_LENGTH/2)) * sizeof(float));
	FMOD_memset(mOutputAccum, 0, 2 * MAX_FRAME_LENGTH * sizeof(float));

  	FMOD_memset(gFFTworksp,   0, 2 * MAX_FRAME_LENGTH * sizeof(float));
	FMOD_memset(gAnaFreq,     0,     MAX_FRAME_LENGTH * sizeof(float));
	FMOD_memset(gAnaMagn,     0,     MAX_FRAME_LENGTH * sizeof(float));
    FMOD_memset(gSynFreq,     0,     MAX_FRAME_LENGTH * sizeof(float));
	FMOD_memset(gSynMagn,     0,     MAX_FRAME_LENGTH * sizeof(float));

    mRover = false;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Macintosh, XBox, PlayStation 2, GameCube

	[SEE_ALSO]
]
*/
// VERIFY: 7454d61f manual
// OFFSET: 0x008EC520
FMOD_RESULT DSPPitchShift::createInternal()
{
    FMOD_RESULT result;
    int count;

    init();

#ifdef DSP_PITCHSHIFT_USECOSTAB
    for (count = 0; count < DSP_PITCHSHIFT_COSTABSIZE; count++)
    {
        mCosTab[count] = (float)FMOD_COS(FMOD_PI_2 * (float)count / (float)DSP_PITCHSHIFT_COSTABSIZE);
    }
#endif

    mOverlap = 4;
    mPitchShift = 0;
    mPitch = 1.0f;
    for (count = 0; count < mDescription.numparameters; count++)
    {
        result = setParameter(count, mDescription.paramdesc[count].defaultval);
        if (result != FMOD_OK)
        {
            return result;
        }
    }
    return FMOD_OK;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Macintosh, XBox, PlayStation 2, GameCube

	[SEE_ALSO]
]
*/
FMOD_RESULT DSPPitchShift::releaseInternal()
{
    if (mPitchShift)
    {
        FMOD_Memory_Free(mPitchShift);
        mPitchShift = 0;
    }

    return FMOD_OK;
}


#ifdef DSP_PITCHSHIFT_USECOSTAB

/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

	[SEE_ALSO]
]
*/
FMOD_INLINE const float DSPPitchShiftSMB::cosine(float x)
{
    int y;

    x *= DSP_PITCHSHIFT_TABLERANGE;
    y = (int)(x);
    if (y < 0)
    {
        y = -y; 
    }

    y &= DSP_PITCHSHIFT_TABLEMASK;
    switch (y >> DSP_PITCHSHIFT_COSTABBITS)
    {
        case 0 : return  mCosTab[y]; 
        case 1 : return -mCosTab[(DSP_PITCHSHIFT_COSTABSIZE - 1) - (y - (DSP_PITCHSHIFT_COSTABSIZE * 1))]; 
        case 2 : return -mCosTab[                                  (y - (DSP_PITCHSHIFT_COSTABSIZE * 2))]; 
        case 3 : return  mCosTab[(DSP_PITCHSHIFT_COSTABSIZE - 1) - (y - (DSP_PITCHSHIFT_COSTABSIZE * 3))]; 
    }

    return 0.0f;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

	[SEE_ALSO]
]
*/
FMOD_INLINE const float DSPPitchShiftSMB::sine(float x)
{
    return cosine(x - 0.25f);
}

#endif


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]
	Routine smbPitchShift(). See top of file for explanation
	Purpose: doing pitch shifting while maintaining duration using the Short
	Time Fourier Transform.
	Author: (c)1999-2002 Stephan M. Bernsee <smb@dspdimension.com>

    [PLATFORMS]
    Win32, Win64, Linux, Macintosh, XBox, PlayStation 2, GameCube

	[SEE_ALSO]
]
*/


#ifdef USEWINDOWTAB

static float vwin1024[513] = { 
  0,
  0.0000094123586994454555565426F, 
  0.0000376490804277485047180107F, 
  0.0000847091020882984047091213F, 
  0.0001505906518978750163739733F, 
  0.0002352912494534287191072508F, 
  0.000338807705825228122620274F, 
  0.0004611361236773192651128284F, 
  0.0006022718974137974967675291F, 
  0.0007622097133526128942548894F, 
  0.0009409435499254104051658487F, 
  0.0011384666779041818784889983F, 
  0.0013547716606548965145861985F, 
  0.0015898503544171660450956551F, 
  0.001843693908610999354635851F, 
  0.0021162927661700914327980172F, 
  0.0024076366639015356341246843F, 
  0.0027177146328722923129816991F, 
  0.0030465149988219697441138578F, 
  0.0033940253826027499961526246F, 
  0.0037602327006450164681439219F, 
  0.0041451231654502374013304689F, 
  0.004548682286109995143164042F, 
  0.0049708948688514387193038147F, 
  0.0054117450176094927805081625F, 
  0.005871216134625267812907623F, 
  0.0063492909210707826339614712F, 
  0.0068459513777006653079126863F, 
  0.0073611788055293891908092974F, 
  0.0078949538065354873950241199F, 
  0.0084472562843918574948531841F, 
  0.0090180654452223785177977788F, 
  0.0096073597983847847103788808F, 
  0.0102151171572797405673327376F, 
  0.0108413146401861726353388349F, 
  0.0114859286711228025801290187F, 
  0.0121489349807357149835240762F, 
  0.0128303086072120708927002397F, 
  0.0135300238972199116105343819F, 
  0.014248054506874108238179133F, 
  0.0149843734027280128806580706F, 
  0.0157389528627913111158420634F, 
  0.0165117644775739647045043057F, 
  0.017302779151155300851883112F, 
  0.0181119671022800798887431029F, 
  0.0189392978654792099035830688F, 
  0.0197847402922171067274348388F, 
  0.0206482625520642004701699079F, 
  0.0215298321338955878090359874F, 
  0.0224294158471146087840963901F, 
  0.0233469798229030689462604187F, 
  0.0242824895154958309007042772F, 
  0.0252359097034816626248243665F, 
  0.0262072044911294543823032654F, 
  0.027196337309739360144078546F, 
  0.0282032709190198072057853551F, 
  0.0292279674084895968455555249F, 
  0.0302703881989050405110219799F, 
  0.0313304940437125201135870611F, 
  0.0324082450305261948741986089F, 
  0.0335036005826305216537264187F, 
  0.0346165194605081438794513815F, 
  0.0357469597633922053780963779F, 
  0.0368948789308443103607260127F, 
  0.0380602337443566307584319475F, 
  0.0392429803289789935760722983F, 
  0.0404430741549711147975187941F, 
  0.0416604700394786475747821441F, 
  0.0428951221482346545244013214F, 
  0.0441469839972850608411647499F, 
  0.0454160084547388098741294016F, 
  0.0467021477425423325868791835F, 
  0.048005353438278330902022617F, 
  0.0493255764769889859522322695F, 
  0.0506627671530230916374648586F, 
  0.0520168751219073910441181852F, 
  0.0533878494022423377707298187F, 
  0.0547756383776210609148904496F, 
  0.0561801897985730325224551507F, 
  0.0576014507845311052314229983F, 
  0.0590393678258224752219973652F, 
  0.060493886785683237405919499F, 
  0.0619649529022966993885290776F, 
  0.063452510790854954603190663F, 
  0.0649565044456442697295983635F, 
  0.0664768772421536757732951628F, 
  0.0680135719392065962729532203F, 
  0.0695665306811163453026836123F, 
  0.0711356949998639409571410397F, 
  0.0727210058172997331205067439F, 
  0.0743224034473674022294176211F, 
  0.0759398275983513837417149261F, 
  0.077573217375146441554534249F, 
  0.0792225112815507781505175444F, 
  0.0808876472225809606264590457F, 
  0.0825685625068099948720146131F, 
  0.0842651938487273821642986604F, 
  0.0859774773711221018679395911F, 
  0.087705348607487354506417887F, 
  0.0894487425044476758273503947F, 
  0.0912075934242081443059646517F, 
  0.0929818351470257931090657166F, 
  0.0947714008737026158968319578F, 
  0.0965762232281003329958934955F, 
  0.0983962342596775285663568411F, 
  0.1002313654460474934282387949F, 
  0.1020815476955582168372416163F, 
  0.1039467113498938055649034595F, 
  0.1058267861866968306827629931F, 
  0.1077217014222123792066554415F, 
  0.1096313857139527558892666548F, 
  0.111555767163383778850516137F, 
  0.1134947733186315033115931783F, 
  0.1154483311772101505887633266F, 
  0.1174163671887705207019791942F, 
  0.1193988072578691106429005231F, 
  0.1213955767467577162577185845F, 
  0.1234066004781937397893898378F, 
  0.1254318027382703149008591481F, 
  0.1274711072792669708242385695F, 
  0.1295244373225204470578830751F, 
  0.1315917155613150479886996891F, 
  0.1336728641637935921515634163F, 
  0.1357678047758874018136054929F, 
  0.1378764585242664986175498143F, 
  0.1399987460193091726168290734F, 
  0.1421345873580907026578756813F, 
  0.1442839021273917832210997858F, 
  0.1464466094067262136313445353F, 
  0.1486226277713873500374575087F, 
  0.150811875295513542205583235F, 
  0.1530142695551729992153866533F, 
  0.1552297276314665297469730376F, 
  0.1574581661136498222930413249F, 
  0.1596995011022734334282802138F, 
  0.1619536482123419829370902789F, 
  0.1642205225764908349539439314F, 
  0.1665000388481812643171053878F, 
  0.1687921112049141081357106486F, 
  0.1710966533514606813248803974F, 
  0.1734135785231116222426805962F, 
  0.175742799488943723940082009F, 
  0.1780842285551042514235575709F, 
  0.1804377775681121343076540597F, 
  0.182803357918177256102865158F, 
  0.1851808805425364523600251232F, 
  0.1875702559288067727827353792F, 
  0.1899713941183553966851604855F, 
  0.1923842047096865903732521019F, 
  0.1948085968618452623601911F, 
  0.1972444792978372274383502827F, 
  0.1996917603080655134739629375F, 
  0.202150347753783266036009536F, 
  0.2046201490705628622812639605F, 
  0.2071010712717805679616844827F, 
  0.2095930209521177367548716575F, 
  0.2120959042910773306722660436F, 
  0.2146096270565163166565980646F, 
  0.217134094608193384257077696F, 
  0.2196692119013319843823239808F, 
  0.2222148834901988556644880646F, 
  0.2247710135316975943453599029F, 
  0.2273375057889767680840975572F, 
  0.2299142636350535173761500118F, 
  0.2325011900564513678268951935F, 
  0.2350981876568525863469005799F, 
  0.2377051586607655808691674792F, 
  0.2403220049172052341646121931F, 
  0.24294862790338916935795055F, 
  0.2455849287284465054526094718F, 
  0.2482308081371412122884123619F, 
  0.25088616651360906573131615F, 
  0.2535509038851079255394438405F, 
  0.2562249199257820020392273364F, 
  0.2589081139604385572994260656F, 
  0.2616003849683388726710120409F, 
  0.2643016315870010957134184082F, 
  0.2670117521160169093974445786F, 
  0.2697306445208800251833736183F, 
  0.2724582064368280542865363714F, 
  0.2751943351726967024184489219F, 
  0.2779389277147853443139524643F, 
  0.2806918807307361429792536001F, 
  0.2834530905734239936144547301F, 
  0.2862224532848589020339602484F, 
  0.2889998646001000759397925322F, 
  0.2917852199511813404697591068F, 
  0.294578414471048044553924683F, 
  0.2973793429975050695013294444F, 
  0.3001879000771766059507683622F, 
  0.303003979969475922828792136F, 
  0.3058274766505868491606179305F, 
  0.3086582838174550813548080441F, 
  0.3114962948917908702739509863F, 
  0.3143414030240812007122030991F, 
  0.3171935010976130175919251997F, 
  0.3200524817325058890560285363F, 
  0.3229182372897547725898448334F, 
  0.3257906598752827731502179631F, 
  0.3286696413440027830787926177F, 
  0.3315550733038900022009443092F, 
  0.334446847120061785396671894F, 
  0.3373448539188684813794338879F, 
  0.3402489845919921540584596187F, 
  0.3431591298005541856852573801F, 
  0.3460751799792325389404368252F, 
  0.3489970253403859024032840352F, 
  0.35192455587818805007316314F, 
  0.3548576613727688622290656895F, 
  0.3577962313943641170510545635F, 
  0.3607401553074734978920901085F, 
  0.3636893222750254839326089495F, 
  0.3666436212625507895523924162F, 
  0.3696029410423622429959777946F, 
  0.3725671701977426586438468803F, 
  0.3755361971271398702221233634F, 
  0.378509910048367981261208115F, 
  0.3814881970028163316044356179F, 
  0.3844709458596643480987609109F, 
  0.3874580443201036117351065968F, 
  0.3904493799215651428369255882F, 
  0.3934448400419543467876337672F, 
  0.3964443119038907337170485334F, 
  0.3994476825789540219702189461F, 
  0.4024548389919358482025302237F, 
  0.4054656679250968620564776757F, 
  0.4084800560224294829758662218F, 
  0.4114978897939255420013182629F, 
  0.4145190556198493081474509836F, 
  0.4175434397550149556721521549F, 
  0.4205709283330693049052229071F, 
  0.4236014073707782823241529968F, 
  0.4266347627723191537896241243F, 
  0.4296708803335753668051211207F, 
  0.432709645746436888380515029F, 
  0.4357509446031033739643589797F, 
  0.4387946624003918882905850296F, 
  0.4418406845440475683162162568F, 
  0.4448888963530583939842699692F, 
  0.4479391830639726235219200134F, 
  0.45099142983521961491888419F, 
  0.4540455217514336450079781571F, 
  0.4571013438277800600140210463F, 
  0.4601587810142849233052686486F, 
  0.4632177182001662729682323061F, 
  0.4662780402181679328954544417F, 
  0.469339631848895655341635802F, 
  0.4724023778251549843254508687F, 
  0.4754661628362909508993539021F, 
  0.4785308715325295447762243839F, 
  0.4815963885293205182236420114F, 
  0.4846625984116816887592449348F, 
  0.4877293857385438524687515383F, 
  0.4907966350470975847031240846F, 
  0.4938642308571400407757323592F, 
  0.4969320576754227558602394765F,
  0.5F, 
  0.5030679423245771886286092922F, 
  0.5061357691428599592242676408F, 
  0.5092033649529023042745734529F, 
  0.5122706142614560365089459992F, 
  0.5153374015883182002184526027F, 
  0.5184036114706794817763579886F, 
  0.5214691284674703997126243848F, 
  0.5245338371637090491006460979F, 
  0.5275976221748449601633979F, 
  0.5306603681511042891472129668F, 
  0.5337219597818320115933943271F, 
  0.5367822817998336715206164627F, 
  0.5398412189857150211835801201F, 
  0.5428986561722198844748277224F, 
  0.5459544782485662439697193804F, 
  0.5490085701647803295699645787F, 
  0.552060816936027265455777524F, 
  0.5551111036469414949934275683F, 
  0.5581593154559523206614812807F, 
  0.5612053375996081117094149704F, 
  0.5642490553968965150133385578F, 
  0.5672903542535630005971825085F, 
  0.5703291196664245221725764168F, 
  0.5733652372276808462103758757F, 
  0.5763985926292216621646957719F, 
  0.5794290716669305840724746304F, 
  0.5824565602449849333055453826F, 
  0.5854809443801506363413977851F, 
  0.5885021102060743469763792746F, 
  0.591519943977570461512982547F, 
  0.5945343320749030269212198618F, 
  0.597545161008064096286318545F, 
  0.6005523174210458670074785914F, 
  0.6035556880961092662829514666F, 
  0.6065551599580456532123662328F, 
  0.6095506200784348571630744118F, 
  0.6125419556798963327537421719F, 
  0.6155290541403355408789366265F, 
  0.6185118029971836683955643821F, 
  0.6214900899516319077164894225F, 
  0.6244638028728600742667254053F, 
  0.6274328298022572303338506572F, 
  0.6303970589576377570040222054F, 
  0.6333563787374492104476075838F, 
  0.6363106777249744050450885879F, 
  0.6392598446925265021079098915F, 
  0.6422037686056358829489454365F, 
  0.645142338627231026748631848F, 
  0.64807544412181194992683686F, 
  0.6510029746596139865744135022F, 
  0.6539248200207674610595631748F, 
  0.6568408701994457032924401574F, 
  0.6597510154080078459415403813F, 
  0.6626551460811314075982636496F, 
  0.6655531528799381035810256435F, 
  0.6684449266961099977990556908F, 
  0.6713303586559971058989049197F, 
  0.6742093401247172268497820369F, 
  0.6770817627102452274101551666F, 
  0.6799475182674941109439714637F, 
  0.6828064989023869824080748003F, 
  0.6856585969759187992877969009F, 
  0.6885037051082091297260490137F, 
  0.6913417161825448076228894934F, 
  0.6941725233494131508393820695F, 
  0.6969960200305239661489054015F, 
  0.6998120999228233385380804066F, 
  0.7026206570024948749875193243F, 
  0.7054215855289518444237728545F, 
  0.7082147800488185485079384307F, 
  0.7110001353998998130379050053F, 
  0.7137775467151409314325860578F, 
  0.7165469094265759508743940387F, 
  0.7193081192692636349761414749F, 
  0.7220610722852145446637450732F, 
  0.7248056648273033530927023094F, 
  0.7275417935631718346911611661F, 
  0.7302693554791199748166263817F, 
  0.7329882478839829795802529588F, 
  0.7356983684129988487754303605F, 
  0.7383996150316609607955342653F, 
  0.7410918860395613316782714719F, 
  0.7437750800742178869384702011F, 
  0.7464490961148919634382536969F, 
  0.7491138334863908232463813874F, 
  0.7517691918628587322004364069F, 
  0.7544150712715536055696929907F, 
  0.7570513720966107751308982188F, 
  0.7596779950827947658353878069F, 
  0.7622948413392343081085300582F, 
  0.7649018123431473581419481889F, 
  0.7674988099435484656396511127F, 
  0.7700857363649464826238499882F, 
  0.7726624942110231764047512115F, 
  0.7752289864683024056546400971F, 
  0.7777851165098009778020582417F, 
  0.780330788098667960106524788F, 
  0.7828659053918065602317710727F, 
  0.7853903729434835723210994729F, 
  0.7879040957089227248388851876F, 
  0.7904069790478822632451283425F, 
  0.7928989287282194320383155173F, 
  0.7953798509294369711852823457F, 
  0.7978496522462166229416880014F, 
  0.8003082396919343199925833687F, 
  0.8027555207021627170504984861F, 
  0.8051914031381548486621113625F, 
  0.8076157952903133541155966668F, 
  0.8100286058816446033148395145F, 
  0.8124297440711931717061133895F, 
  0.8148191194574635476399748768F, 
  0.8171966420818226328748323795F, 
  0.8195622224318879212034971715F, 
  0.8219157714448956930652911979F, 
  0.8242572005110562205487667597F, 
  0.8265864214768883222461681726F, 
  0.8289033466485393741862708339F, 
  0.8312078887950860028865918139F, 
  0.8334999611518186801717433809F, 
  0.8357794774235092205572072999F, 
  0.8380463517876579615517584898F, 
  0.840300498897726511060568555F, 
  0.8425418338863501777069586751F, 
  0.8447702723685335257641781936F, 
  0.8469857304448269452734621154F, 
  0.8491881247044863467721143024F, 
  0.8513773722286125389402400288F, 
  0.8535533905932737308575042334F, 
  0.8557160978726082722900514455F, 
  0.8578654126419092973421243187F, 
  0.8600012539806908273831709266F, 
  0.8621235414757333348489964919F, 
  0.8642321952241125426752432759F, 
  0.8663271358362063523372853524F, 
  0.8684082844386848965001490797F, 
  0.8704755626774793864086632311F, 
  0.8725288927207329736646101992F, 
  0.8745681972617296295879896206F, 
  0.8765933995218062602106101622F, 
  0.8786044232532422837422814155F, 
  0.8806011927421308893570994769F, 
  0.8825836328112295348091720371F, 
  0.8845516688227896828777829796F, 
  0.8865052266813684411772555904F, 
  0.888444232836616221149483863F, 
  0.890368614286047188599582114F, 
  0.892278298577787509771042096F, 
  0.8941732138133031693172370069F, 
  0.8960532886501060279016428467F, 
  0.8979184523044416721404559212F, 
  0.8997686345539526175940636676F, 
  0.9016037657403224159224919276F, 
  0.9034237767718996670041065045F, 
  0.9052285991262973841031680422F, 
  0.9070181648529742624020855146F, 
  0.9087924065757917446717328858F, 
  0.9105512574955523241726496053F, 
  0.9122946513925125344712796505F, 
  0.9140225226288778426209091776F, 
  0.9157348061512726733468525708F, 
  0.9174314374931900051279853869F, 
  0.9191123527774189838623897231F, 
  0.9207774887184492218494824556F, 
  0.922426782624853558445465751F, 
  0.9240601724016486162582850739F, 
  0.9256775965526325977705823789F, 
  0.9272789941827002113683420248F, 
  0.9288643050001359480205564978F, 
  0.9304334693188835991861651564F, 
  0.9319864280607933482158955485F, 
  0.9335231227578464352490072997F, 
  0.935043495554355619248099174F, 
  0.9365474892091449898856581058F, 
  0.9380350470977032451003196911F, 
  0.939506113214316762594080501F, 
  0.9409606321741774692668514035F, 
  0.942398549215468950279728233F, 
  0.9438198102014269119663936181F, 
  0.9452243616223789945962607817F, 
  0.9466121505977576067181189501F, 
  0.9479831248780925534447305836F, 
  0.9493372328469769083625351414F, 
  0.950674423523010903025465268F, 
  0.9519946465617217246091286142F, 
  0.9532978522574576674131208165F, 
  0.9545839915452611901258705984F, 
  0.9558530160027148836476840188F, 
  0.9571048778517653454755986786F, 
  0.9583395299605212969140666246F, 
  0.9595569258450289407136324371F, 
  0.9607570196710208954016252392F, 
  0.9619397662556433692415680525F, 
  0.9631051210691556896392739873F, 
  0.9642530402366077391107523908F, 
  0.9653834805394918561205486185F, 
  0.9664963994173694228351223501F, 
  0.9675917549694738051258013911F, 
  0.9686695059562874243752617076F, 
  0.9697296118010949594889780201F, 
  0.9707720325915103476432932439F, 
  0.9717967290809801372830634136F, 
  0.9728036626902605288336189915F, 
  0.9737927955088705456176967346F, 
  0.9747640902965183373751756335F, 
  0.9757175104845041690992957228F, 
  0.9766530201770968755425883501F, 
  0.9775705841528853357047523787F, 
  0.9784701678661044121909640126F, 
  0.9793517374479356885075276296F, 
  0.9802152597077828932725651612F, 
  0.9810607021345207900964169312F, 
  0.9818880328977199756224081284F, 
  0.982697220848844699148116888F, 
  0.9834882355224260352954956943F, 
  0.9842610471372086333730067054F, 
  0.9850156265972720426304931607F, 
  0.9857519454931258362506696358F, 
  0.9864699761027799773671631556F, 
  0.987169691392787873596148529F, 
  0.9878510650192642295053246926F, 
  0.9885140713288771419087197501F, 
  0.9891586853598137718535099339F, 
  0.9897848828427202594326672624F, 
  0.9903926402016152152896211192F, 
  0.9909819345547776769933534524F, 
  0.9915527437156081980162980472F, 
  0.9921050461934645126049758801F, 
  0.9926388211944705552980394714F, 
  0.9931540486222992791809360824F, 
  0.99365070907892927287718976F, 
  0.994128783865374732187092377F, 
  0.9945882549823905627306430688F, 
  0.9950291051311485057695449541F, 
  0.9954513177138899493456847267F, 
  0.9958548768345497625986695311F, 
  0.9962397672993550390430073094F, 
  0.9966059746173971944926961442F, 
  0.9969534850011780857670373734F, 
  0.9972822853671277076870183009F, 
  0.9975923633360983533435728532F, 
  0.9978837072338299085672019828F, 
  0.9981563060913889451342129178F, 
  0.9984101496455828339549043449F, 
  0.9986452283393451034854138015F, 
  0.9988615333220958181215110017F, 
  0.99905905645007453408368292F, 
  0.9992377902866473871057451106F, 
  0.9993977281025862025032324709F, 
  0.9995388638763227362460384029F, 
  0.9996611922941747163662284947F, 
  0.9997647087505465712808927492F, 
  0.9998494093481020694724747955F, 
  0.9999152908979116460841396474F, 
  0.9999623509195723070064332205F, 
  0.9999905876413005545444434574F,
  1.0F
 };

#endif

#define kINVPI  0.3183098861837F
#define kINVPI2 0.1591549430918F


// VERIFY: eed8bf26 manual
// OFFSET: 0x008ECF60
void DSPPitchShiftSMB::smbPitchShift(float pitchShift, int numSampsToProcess, int osamp, float sampleRate, float *indata, float *outdata, int channel, int numchannels)
{
	float   window;
	float   freqPerBin, expct;
	int     i,k, index, inFifoLatency, stepSize, fftFrameSize2, fadeZoneLen;
    float  *inptr, sum;
  
    
    inptr = indata + channel;
    sum = 0;
	for (i = 0; i < numSampsToProcess; i++)
    {
        sum += FMOD_FABS(inptr[0]);
        inptr += numchannels;
    }
    if (sum < .001f)
    {
        float *outptr;
        /*
            Silence anyway, so clear outdata to silence.
        */
        outptr = outdata + channel;
        
	    for (i = 0; i < numSampsToProcess; i++)
        {
            outptr[0] = 0;            
            outptr += numchannels;
        }
        return;
    }
  
  
	/* 
        Set up some handy variables 
    */
	fadeZoneLen     = mFFTFrameSize/2;
	fftFrameSize2   = mFFTFrameSize/2;
	stepSize        = mFFTFrameSize/osamp;
	freqPerBin      = sampleRate/(float)mFFTFrameSize;
	expct           = FMOD_PI2 * (float)stepSize/(float)mFFTFrameSize;
	inFifoLatency   = mFFTFrameSize-stepSize;


    if (mRover == false) 
    {
        mRover = inFifoLatency;
    }

    if (mResetPhaseFlag)
    {
        //
        // Best solution so far seems a simple re-init to zero
        //
            /* PDT - experiment to correct phase data when user changes pitch shift ratio.
           for(int sample = 0; sample <= (MAX_FRAME_LENGTH/2); sample++)
            {
               mPitchShift[count].mLastPhase[sample] = 0.0f;
               mPitchShift[count].mSumPhase[sample] = 0.0f; //mPitchShift[count].mLastPhaseWithShift[sample];
            }
            */
        FMOD_memset( mLastPhase, 0, (4+(MAX_FRAME_LENGTH/2)) * sizeof(float));
        FMOD_memset( mSumPhase, 0, (4+(MAX_FRAME_LENGTH/2)) * sizeof(float));
        mResetPhaseFlag = false;
    }

    /* 
        main processing loop 
    */
	for (i = 0; i < numSampsToProcess; i++)
    {

		/* As long as we have not yet collected enough data just read in */
		mInFIFO[mRover] = indata[i * numchannels + channel];
		outdata[i * numchannels + channel] = mOutFIFO[mRover-inFifoLatency];
		mRover++;

		/* now we have enough data for processing */
		if (mRover >= mFFTFrameSize) 
        {
			mRover = inFifoLatency;

            //
            // Window the input data. Direct lookup is possible
            //
			for (k = 0; k < mFFTFrameSize; k++) 
            {
            #ifdef USEWINDOWTAB
                window = mWindow[k];
            #else
                #ifdef DSP_PITCHSHIFT_USECOSTAB
                window = -0.5f * cosine((float)k/(float)mFFTFrameSize)+0.5f;
                #else
                window = -0.5f * cosine(FMOD_PI2 * (float)k/(float)mFFTFrameSize)+0.5f;
                #endif
            #endif

				gFFTworksp[2*k] = mInFIFO[k] * window;
				gFFTworksp[2*k+1] = 0.0f;
			}

			/* ***************** ANALYSIS ******************* */
			/* do transform */
#ifdef FASTER_DFT
			fft(gFFTworksp, -1);
#else
			smbFft(gFFTworksp, -1);
#endif

            float   phase0, phase1,phase2,phase3;
            float   tmp0, tmp1, tmp2, tmp3;
            float   real0, real1, real2,real3;
            float   imag0, imag1, imag2, imag3 ;
            int     qpd0, qpd1, qpd2, qpd3;
            int     k0, k1, k2, k3;
            float   factor = freqPerBin * osamp/FMOD_PI2;

			/* this is the analysis step */
			for (k = 0; k <= fftFrameSize2; k+=4) 
            {
                k0 = k;
                k1 = k+1;
                k2 = k+2;
                k3 = k+3;

                /* de-interlace FFT buffer */
				real0 = gFFTworksp[2*k0];
				imag0 = gFFTworksp[2*k0+1];
				real1 = gFFTworksp[2*k1];
				imag1 = gFFTworksp[2*k1+1];
				real2 = gFFTworksp[2*k2];
				imag2 = gFFTworksp[2*k2+1];
				real3 = gFFTworksp[2*k3];
				imag3 = gFFTworksp[2*k3+1];

                // Kick phase calculations off early
                //
                phase0 = smbAtan2(imag0,real0);
				phase1 = smbAtan2(imag1,real1);
				phase2 = smbAtan2(imag2,real2);
				phase3 = smbAtan2(imag3,real3);

				// Meanwhile calculate magnitudes
#if 0
				gAnaMagn[k0] = 2.0f*FMOD_SQRT(real0*real0 + imag0*imag0);
				gAnaMagn[k1] = 2.0f*FMOD_SQRT(real1*real1 + imag1*imag1);
				gAnaMagn[k2] = 2.0f*FMOD_SQRT(real2*real2 + imag2*imag2);
				gAnaMagn[k3] = 2.0f*FMOD_SQRT(real3*real3 + imag3*imag3);
#else


                FMOD_VECTOR v0 = { real0, imag0, 0};
                gAnaMagn[k0] = 2.0f*FMOD_Vector_GetLengthFast(&v0);
                FMOD_VECTOR v1 = { real1, imag1, 0};
                gAnaMagn[k1] = 2.0f*FMOD_Vector_GetLengthFast(&v1);
                FMOD_VECTOR v2 = { real2, imag2, 0};
                gAnaMagn[k2] = 2.0f*FMOD_Vector_GetLengthFast(&v2);
                FMOD_VECTOR v3 = { real3, imag3, 0};
                gAnaMagn[k3] = 2.0f*FMOD_Vector_GetLengthFast(&v3);

#endif
                
                // Back to phase... initialise the temporary variables

				tmp0 = -((float)(k0)*expct + mLastPhase[k0]);
				tmp1 = -((float)(k1)*expct + mLastPhase[k1]);
				tmp2 = -((float)(k2)*expct + mLastPhase[k2]);
				tmp3 = -((float)(k3)*expct + mLastPhase[k3]);

                // Add calculated phase
                tmp0 += phase0;
				mLastPhase[k0] = phase0;
				tmp1 += phase1;
				mLastPhase[k1] = phase1;
				tmp2 += phase2;
				mLastPhase[k2] = phase2;
				tmp3 += phase3;
				mLastPhase[k3] = phase3;

				/* map delta phase into +/- Pi interval */
				qpd0 = (int)(tmp0 * kINVPI);
				if (qpd0 >= 0) qpd0 += qpd0&1;
				else qpd0 -= qpd0&1;
				tmp0 -= FMOD_PI * (float)qpd0;

				qpd1 = (int)(tmp1 * kINVPI);
				if (qpd1 >= 0) qpd1 += qpd1&1;
				else qpd1 -= qpd1&1;
				tmp1 -= FMOD_PI * (float)qpd1;

				qpd2 = (int)(tmp2 * kINVPI);
				if (qpd2 >= 0) qpd2 += qpd2&1;
				else qpd2 -= qpd2&1;
				tmp2 -= FMOD_PI * (float)qpd2;

				qpd3 = (int)(tmp3 * kINVPI);
				if (qpd3 >= 0) qpd3 += qpd3&1;
				else qpd3 -= qpd3&1;
				tmp3 -= FMOD_PI * (float)qpd3;

				/* compute the k-th partials' true frequency */
				gAnaFreq[k0] = (float)k0*freqPerBin + tmp0*factor;
				gAnaFreq[k1] = (float)k1*freqPerBin + tmp1*factor;
				gAnaFreq[k2] = (float)k2*freqPerBin + tmp2*factor;
				gAnaFreq[k3] = (float)k3*freqPerBin + tmp3*factor;

			}

			/* ***************** PROCESSING ******************* */
			/* this does the actual pitch shifting */
			FMOD_memset(gSynMagn, 0, mFFTFrameSize*sizeof(float));
			FMOD_memset(gSynFreq, 0, mFFTFrameSize*sizeof(float));

            // Try to maintain the neighbouring frequencies
			for (k = 0; k <= fftFrameSize2; k++) 
            {
                index = (int)((k*pitchShift)+0.5f);
				if (index <= fftFrameSize2) 
                {
					gSynFreq[index] = gAnaFreq[k]*pitchShift;
					gSynMagn[index] += gAnaMagn[k];
				}
			}

            float phasemul = FMOD_PI2 /osamp;

			/* ***************** SYNTHESIS ******************* */
			/* this is the synthesis step */
			for (k = 0; k <= fftFrameSize2; k+=4) 
            {
                k0 = k;
                k1 = k+1;
                k2 = k+2;
                k3 = k+3;

				/* accumulate delta phase to get bin phase */
				mSumPhase[k0] += k0*expct + phasemul*((gSynFreq[k0] / freqPerBin) - k0);
				mSumPhase[k1] += k1*expct + phasemul*((gSynFreq[k1] / freqPerBin) - k1);
				mSumPhase[k2] += k2*expct + phasemul*((gSynFreq[k2] / freqPerBin) - k2);
				mSumPhase[k3] += k3*expct + phasemul*((gSynFreq[k3] / freqPerBin) - k3);

                if(mSumPhase[k0] > FMOD_PI)
                   mSumPhase[k0] -= ((int)(mSumPhase[k0]*kINVPI2)) * FMOD_PI2;
                else if(mSumPhase[k0] < -FMOD_PI)
                    mSumPhase[k0] -= ((int)(mSumPhase[k0]*kINVPI2)) * FMOD_PI2;

                if(mSumPhase[k1] > FMOD_PI)
                   mSumPhase[k1] -= ((int)(mSumPhase[k1]*kINVPI2)) * FMOD_PI2;
                else if(mSumPhase[k1] < -FMOD_PI)
                    mSumPhase[k1] -= ((int)(mSumPhase[k1]*kINVPI2)) * FMOD_PI2;

                if(mSumPhase[k2] > FMOD_PI)
                   mSumPhase[k2] -= ((int)(mSumPhase[k2]*kINVPI2)) * FMOD_PI2;
                else if(mSumPhase[k2] < -FMOD_PI)
                    mSumPhase[k2] -= ((int)(mSumPhase[k2]*kINVPI2)) * FMOD_PI2;

                if(mSumPhase[k3] > FMOD_PI)
                   mSumPhase[k3] -= ((int)(mSumPhase[k3]*kINVPI2)) * FMOD_PI2;
                else if(mSumPhase[k3] < -FMOD_PI)
                    mSumPhase[k3] -= ((int)(mSumPhase[k3]*kINVPI2)) * FMOD_PI2;

#ifdef DSP_PITCHSHIFT_USECOSTAB
                phase0 = mSumPhase[k0] * kINVPI2;
                phase1 = mSumPhase[k1] * kINVPI2;
                phase2 = mSumPhase[k2] * kINVPI2;
                phase3 = mSumPhase[k3] * kINVPI2;
#else
                phase0 = mSumPhase[k0];
                phase1 = mSumPhase[k1];
                phase2 = mSumPhase[k2];
                phase3 = mSumPhase[k3];
#endif

				/* get real and imag part and re-interleave */
				gFFTworksp[2*k0]   = gSynMagn[k0] * cosine(phase0);
				gFFTworksp[2*k0+1] = gSynMagn[k0] *   sine(phase0);
				gFFTworksp[2*k1]   = gSynMagn[k1] * cosine(phase1);
				gFFTworksp[2*k1+1] = gSynMagn[k1] *   sine(phase1);
				gFFTworksp[2*k2]   = gSynMagn[k2] * cosine(phase2);
				gFFTworksp[2*k2+1] = gSynMagn[k2] *   sine(phase2);
				gFFTworksp[2*k3]   = gSynMagn[k3] * cosine(phase3);
				gFFTworksp[2*k3+1] = gSynMagn[k3] *   sine(phase3);

			} 
			/* zero negative frequencies */
            FMOD_memset(&gFFTworksp[mFFTFrameSize+2], 0, sizeof(float)*(mFFTFrameSize - 2));
			// for (k = mFFTFrameSize+2; k < 2*mFFTFrameSize; k++) gFFTworksp[k] = 0.0f;

			/* do inverse transform */

#ifdef FASTER_DFT
			fft(gFFTworksp, 1);
#else
			smbFft(gFFTworksp, 1);
#endif

#ifdef USEWINDOWTAB
            for(k=0; k < mFFTFrameSize; k++) 
            {
                //mOutputAccum[k] += 2.0f*gWindow[k]*gFFTworksp[2*k]/(fftFrameSize2*osamp);

                // Precalculated window includes the root of 4/(fftsize*osamp)
                // (use the root because the window coeffs multiply in both fft and ifft
                //
                mOutputAccum[k] += mWindow[k]*gFFTworksp[2*k];
            }      
#else
			for(k=0; k < mFFTFrameSize; k++) 
            {
                #ifdef DSP_PITCHSHIFT_USECOSTAB
                window = -0.5f * cosine((float)k/(float)mFFTFrameSize)+0.5f;
                #else
                window = -0.5f * cosine(FMOD_PI2 * (float)k/(float)mFFTFrameSize)+0.5f;
                #endif

            	mOutputAccum[k] += window*gFFTworksp[2*k]2.0f/(fftFrameSize2*osamp);
			}
#endif

			for (k = 0; k < stepSize; k++) mOutFIFO[k] = mOutputAccum[k];

			/* shift accumulator */
			// memmove(mOutputAccum, mOutputAccum+stepSize, mFFTFrameSize*sizeof(float));
            // PDT output accumulator init new space at end
			memmove(mOutputAccum, mOutputAccum+stepSize, inFifoLatency*sizeof(float));
            FMOD_memset(mOutputAccum+inFifoLatency,0, stepSize * sizeof(float));


			/* move input FIFO */
			for (k = 0; k < inFifoLatency; k++) mInFIFO[k] = mInFIFO[k+stepSize];
		}
	}
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]
	FFT routine, (C)1996 S.M.Bernsee. Sign = -1 is FFT, 1 is iFFT (inverse)
	Fills fftBuffer[0...2*fftFrameSize-1] with the Fourier transform of the
	time domain data in fftBuffer[0...2*fftFrameSize-1]. The FFT array takes
	and returns the cosine and sine parts in an interleaved manner, ie.
	fftBuffer[0] = cosPart[0], fftBuffer[1] = sinPart[0], asf. fftFrameSize
	must be a power of 2. It expects a complex input signal (see footnote 2),
	ie. when working with 'common' audio signals our input signal has to be
	passed as {in[0],0.,in[1],0.,in[2],0.,...} asf. In that case, the transform
	of the frequencies of interest is in fftBuffer[0...fftFrameSize].

    [PLATFORMS]
    Win32, Win64, Linux, Macintosh, XBox, PlayStation 2, GameCube

	[SEE_ALSO]
]
*/
// VERIFY: 9b0ac4bd manual
// OFFSET: 0x008EC5C0
void DSPPitchShiftSMB::smbFft(float *fftBuffer, int sign)
{
	float wr, wi, arg, *p1, *p2, temp;
	float tr, ti, ur, ui, *p1r, *p1i, *p2r, *p2i;
	int i, bitm, j, le, le2, k;

	for (i = 2; i < 2 * mFFTFrameSize-2; i += 2) 
    {
		for (bitm = 2, j = 0; bitm < 2*mFFTFrameSize; bitm <<= 1) 
        {
			if (i & bitm) j++;
			j <<= 1;
		}
		if (i < j) 
        {
			p1 = fftBuffer+i; 
			p2 = fftBuffer+j;
			
			temp = p1[0]; 
			p1[0] = p2[0];
			p2[0] = temp; 
			
			temp  = p1[1];
			p1[1] = p2[1]; 
			p2[1] = temp;
			
			p1++;
			p2++;
		}
	}

    for (k = 0, le = 2; k < mFFTFrameBits; k++) 
    {
		le <<= 1;
		le2 = le>>1;
		ur = 1.0f;
		ui = 0.0f;
		
#ifdef DSP_PITCHSHIFT_USECOSTAB
        arg  = 0.5f / (le2>>1);
#else
        arg = FMOD_PI / (le2>>1);
#endif		

		wr =        cosine(arg);
		wi = sign *   sine(arg);
		
        for (j = 0; j < le2; j += 2) 
        {
			p1r = fftBuffer+j; 
			p1i = p1r+1;
			p2r = p1r+le2; 
			p2i = p2r+1;

			for (i = j; i < 2*mFFTFrameSize; i += le) 
            {
				tr = p2r[0] * ur - p2i[0] * ui;
				ti = p2r[0] * ui + p2i[0] * ur;
				
				p2r[0] = p1r[0] - tr; 
				p2i[0] = p1i[0] - ti;
				
				p1r[0] += tr; 
				p1i[0] += ti;

				p1r += le; 
				p1i += le;
				p2r += le; 
				p2i += le;
			}
			tr = ur*wr - ui*wi;
			ui = ur*wi + ui*wr;
			ur = tr;
		}
	}
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Macintosh, XBox, PlayStation 2, GameCube

	[SEE_ALSO]
]
*/
FMOD_INLINE const float DSPPitchShiftSMB::smbAtan2(float x, float y)
{

#if 1
    float signx;
    if (x > 0.0f) signx = 1.0f;
    else signx = -1.0f;

    if (x == 0.0f) return 0.0f;
    if (y == 0.0f) return signx * FMOD_PI / 2.0f;

    return (float)atan2(x, y);
#else
    float coeff_1, coeff_2, abs_x, angle;

    coeff_1 = FMOD_PI/4;
    coeff_2 = 3*coeff_1;
    abs_x = FMOD_FABS(x) + (float)1e-10;      // kludge to prevent 0/0 condition

    if (y >= 0)
    {
        angle = coeff_1 - coeff_1 * ((y - abs_x) / (y + abs_x));
    }
    else
    {        
        angle = coeff_2 - coeff_1 * ((y + abs_x) / (abs_x - y));
    }
    if (x < 0)
    {
        return(-angle);     // negate if in quad III or IV
    }
    else
    {
        return(angle);
    }
#endif
}

void DSPPitchShiftSMB::setResetPhaseFlag()
{
    mResetPhaseFlag=true;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Macintosh, XBox, PlayStation 2, GameCube

	[SEE_ALSO]
]
*/
// VERIFY: dba2c6c3 auto
// OFFSET: 0x008EC8A0
FMOD_RESULT DSPPitchShift::resetInternal()
{
    if (mPitchShift)
    {
        int count;

        for (count = 0; count < mChannels; count++)
        {
            mPitchShift[count].smbInit();

            #ifdef DSP_PITCHSHIFT_USECOSTAB
            mPitchShift[count].mCosTab = mCosTab;
            #endif
        }
    }

    return FMOD_OK;
}



/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Macintosh, XBox, PlayStation 2, GameCube

	[SEE_ALSO]
]
*/

// VERIFY: 313dbdf0 manual
// OFFSET: 0x008EDD90
FMOD_RESULT DSPPitchShift::readInternal(float *inbuffer, float *outbuffer, unsigned int length, int inchannels, int outchannels)
{
    int count;

    if (inchannels > mChannels || !mPitchShift)
    {
        FMOD_memcpy(outbuffer, inbuffer, length * outchannels * sizeof(float));
        return FMOD_OK;
    }
	
	for (count = 0; count < inchannels; count++)
	{
		mPitchShift[count].mFFTFrameSize = mFFTSize;
		mPitchShift[count].mFFTFrameBits = mFFTFrameBits;
		mPitchShift[count].smbPitchShift(mPitch, length, mOverlap, (float)mOutputRate, inbuffer, outbuffer, count, inchannels);
	}

    return FMOD_OK;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Macintosh, XBox, PlayStation 2, GameCube

	[SEE_ALSO]
]
*/
// VERIFY: 8c53b3e9 manual
// OFFSET: 0x008EC900
FMOD_RESULT DSPPitchShift::setParameterInternal(int index, float value)
{
    FMOD_RESULT result;
    int         oldchannels, oldfftsize, oldoverlap;
    float       oldpitch;
    bool        done_lock  = false;

    oldchannels = mChannels;
    oldfftsize  = mFFTSize;
    oldpitch    = mPitch;
    oldoverlap  = mOverlap;

    //
    // Get parameters and constrain
    //
    switch (index)
    {
        case FMOD_DSP_PITCHSHIFT_PITCH:
        {
            mPitch = value;
            if(mPitch == 0.0f)
            {
                mPitch = 1.0f;
            }
            break;
        }
        case FMOD_DSP_PITCHSHIFT_FFTSIZE:
        {
            //
            // Constrain FFT size to powers of 2 between 256 and 4096,
            // rounding up
            //
            if(value > 4096)
            {
                mFFTSize = 4096;
            }            
            else
            {
                int nextfftsize = 256;
                while(nextfftsize <= 4096)
                {
                    if(value <= nextfftsize)
                    {
                        mFFTSize = nextfftsize;
                        break;
                    }
                    nextfftsize <<= 1;
                }
            }
            if(oldfftsize != mFFTSize)
            {
                mSystem->lockDSP();
                done_lock = true;
            }
            break;
        }
        case FMOD_DSP_PITCHSHIFT_OVERLAP:
        {
            mOverlap = 4; //(int)value;
            if(mOverlap < 0)
            {
                mOverlap = 0;
            }
            if(oldoverlap != mOverlap)
            {
                mSystem->lockDSP();
                done_lock = true;
            }
            break;
        }
        case FMOD_DSP_PITCHSHIFT_MAXCHANNELS:
        {
            mMaxChannels = (int)value;
            break;
        }
    }

    //
    // Get output rate and number of channels
    //
    result = mSystem->getSoftwareFormat(&mOutputRate, 0, &mChannels, 0, 0, 0);
    if (result != FMOD_OK)
    {
        if(done_lock)
        {
            mSystem->unlockDSP();
        }
        return result;
    }
    if (mMaxChannels)
    {
        mChannels = mMaxChannels;
        if(oldchannels != mChannels)
        {
            mSystem->lockDSP();
            done_lock = true;
        }
    }

    //
    // Recalculate window coefficients if FFT size has changed
    //
    if (oldfftsize != mFFTSize || oldoverlap != mOverlap)
    {
    #ifdef USEWINDOWTAB
        float   *wintab = NULL;
        int     wtabsize = 0;
        bool    window_parity = false;

        if (!mPitchShift)
        {
            return FMOD_ERR_INVALID_HANDLE;
        }

        switch(mFFTSize)
        {
            case 256:
                wintab = vwin1024;
                wtabsize = 513;
                window_parity = false;
                break;
            case 512:
                wintab = vwin1024;
                wtabsize = 513;
                window_parity = false;
                break;
            case 1024:
                wintab = vwin1024;
                wtabsize = 513;
                window_parity = true;
                break;
            case 2048:
                wintab = vwin1024;
                wtabsize = 513;
                window_parity = false;
                break;
            case 4096:
                wintab = vwin1024;
                wtabsize = 513;
                window_parity = false;
                break;
        }

        int ws2m1 = 2*(wtabsize-1);
        float ws2m1ratio = (float)ws2m1/(float)mFFTSize;
        float window_factor = FMOD_SQRT((float)(mFFTSize * 0.25f * mOverlap));

        for(int k=0; k<mFFTSize; k++)
        {
            if(window_parity)
            {
                if(k >= wtabsize)
                {
                    mWindow[k] = wintab[ws2m1-k];
                }
                else
                {
                    mWindow[k] = wintab[k];
                }
            }
            else
            {
                float ktabpos;
                ktabpos = k * ws2m1ratio ;
                int k_a, k_next;
            
                if(ktabpos > (float)(wtabsize-1))
                {
                    ktabpos = ws2m1-ktabpos;
                    k_next = -1;
                }
                else
                {
                    k_next = +1;
                }

                k_a = (int)ktabpos;
                float win_a = wintab[k_a];

                if((float)k_a != ktabpos)
                {
                    //
                    // Linear interpolation between lookups
                    //
                    float win_b = wintab[k_a + k_next];
                    mWindow[k] = win_a + (ktabpos - (float)k_a) * (win_b - win_a);
                }
                else
                {
                    //
                    // Direct lookup
                    //
                    mWindow[k] = win_a;
                }
            }
            mWindow[k] /= window_factor; 
        }
        for (int count = 0; count < mChannels; count++)
        {
            mPitchShift[count].mWindow = mWindow;
        }
    #endif
        reset();
    }
  
    // 
    // If the number of channels has changed, reinitialise everything
    //
    if (mChannels != oldchannels || !mPitchShift)
    {
        int count;

        if (mPitchShift)
        {
            FMOD_Memory_Free(mPitchShift);
        }

        mPitchShift = (DSPPitchShiftSMB *)FMOD_Memory_Alloc(sizeof(DSPPitchShiftSMB) * mChannels);
        if (!mPitchShift)
        {
            if(done_lock)
            {
                mSystem->unlockDSP();
            }
            return FMOD_ERR_MEMORY;
        }

        for (count = 0; count < mChannels; count++)
        {
            mPitchShift[count].smbInit();

            #ifdef USEWINDOWTAB
            mPitchShift[count].mWindow = mWindow;
            #endif

            #ifdef DSP_PITCHSHIFT_USECOSTAB
            mPitchShift[count].mCosTab = mCosTab;
            #endif
        }
    }

    //
    // If pitch has changed, flag a reset of the phase arrays 
    //
    for (int count = 0; count < mChannels; count++)
    {
        mPitchShift[count].mPitchRatioFromLast = mPitch/oldpitch;
        if (oldpitch != mPitch)
        {
            mPitchShift[count].setResetPhaseFlag();
        }
    }

    //
    // Calculate the number of bits in the FFT size
    //
    {
        int bitslength = mFFTSize;
        
        mFFTFrameBits = 0;
        while (bitslength > 1)
        {
            bitslength >>= 1;
            mFFTFrameBits++;
        }
    }

    if(done_lock)
    {
        mSystem->unlockDSP();
    }

    return FMOD_OK;
}

/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Macintosh, XBox, PlayStation 2, GameCube

	[SEE_ALSO]
]
*/
// VERIFY: 3a41558c auto
// OFFSET: 0x008ECD60
FMOD_RESULT DSPPitchShift::getParameterInternal(int index, float *value, char *valuestr)
{
    switch (index)
    {
        case FMOD_DSP_PITCHSHIFT_PITCH:
        {
            if (value)
            {
                *value = mPitch;
            }
            if (valuestr)
            {
                sprintf(valuestr, "%.02f", mPitch);
            }
            break;
        }
        case FMOD_DSP_PITCHSHIFT_FFTSIZE:
        {
            if (value)
            {
                *value = (float)mFFTSize;
            }
            if (valuestr)
            {
                sprintf(valuestr, "%d", mFFTSize);
            }
            break;
        }
        case FMOD_DSP_PITCHSHIFT_OVERLAP:
        {
            if (value)
            {
                *value = (float)mOverlap;
            }
            if (valuestr)
            {
                sprintf(valuestr, "%d", mOverlap);
            }
            break;
        }
        case FMOD_DSP_PITCHSHIFT_MAXCHANNELS:
        {
            if (value)
            {
                *value = (float)mMaxChannels;
            }
            if (valuestr)
            {
                sprintf(valuestr, "%d", mMaxChannels);
            }
            break;
        }
    }

    return FMOD_OK;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
 
	[REMARKS]

    [PLATFORMS]
 
	[SEE_ALSO]
]
*/



/*
    ==============================================================================================================

    CALLBACK INTERFACE

    ==============================================================================================================
*/


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Macintosh, XBox, PlayStation 2, GameCube

	[SEE_ALSO]
]
*/
// VERIFY: 78f63d65 manual
// OFFSET: 0x008ECE40
FMOD_RESULT F_CALLBACK DSPPitchShift::createCallback(FMOD_DSP_STATE *dsp)
{
    DSPPitchShift *pitchshift = (DSPPitchShift *)dsp;

    return pitchshift->createInternal();
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Macintosh, XBox, PlayStation 2, GameCube

	[SEE_ALSO]
]
*/
// VERIFY: 8b0f1885 manual
// OFFSET: 0x008ECE70
FMOD_RESULT F_CALLBACK DSPPitchShift::releaseCallback(FMOD_DSP_STATE *dsp)
{
    DSPPitchShift *pitchshift = (DSPPitchShift *)dsp;

    return pitchshift->releaseInternal();
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Macintosh, XBox, PlayStation 2, GameCube

	[SEE_ALSO]
]
*/
// VERIFY: f552517e manual
// OFFSET: 0x008ECEC0
FMOD_RESULT F_CALLBACK DSPPitchShift::resetCallback(FMOD_DSP_STATE *dsp)
{
    DSPPitchShift *pitchshift = (DSPPitchShift *)dsp;

    return pitchshift->resetInternal();
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Macintosh, XBox, PlayStation 2, GameCube

	[SEE_ALSO]
]
*/
// VERIFY: eccd0176 auto
// OFFSET: 0x008EDE60
FMOD_RESULT F_CALLBACK DSPPitchShift::readCallback(FMOD_DSP_STATE *dsp, float *inbuffer, float *outbuffer, unsigned int length, int inchannels, int outchannels)
{
    DSPPitchShift *pitchshift = (DSPPitchShift *)dsp;

    return pitchshift->readInternal(inbuffer, outbuffer, length, inchannels, outchannels);
}



/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Macintosh, XBox, PlayStation 2, GameCube

	[SEE_ALSO]
]
*/
// VERIFY: 260ee007 manual
// OFFSET: 0x008ECEF0
FMOD_RESULT F_CALLBACK DSPPitchShift::setParameterCallback(FMOD_DSP_STATE *dsp, int index, float value)
{
    DSPPitchShift *pitchshift = (DSPPitchShift *)dsp;

    return pitchshift->setParameterInternal(index, value);
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Macintosh, XBox, PlayStation 2, GameCube

	[SEE_ALSO]
]
*/
// VERIFY: 3967643d auto
// OFFSET: 0x008ECF30
FMOD_RESULT F_CALLBACK DSPPitchShift::getParameterCallback(FMOD_DSP_STATE *dsp, int index, float *value, char *valuestr)
{
    DSPPitchShift *pitchshift = (DSPPitchShift *)dsp;

    return pitchshift->getParameterInternal(index, value, valuestr);
}



}

#endif
