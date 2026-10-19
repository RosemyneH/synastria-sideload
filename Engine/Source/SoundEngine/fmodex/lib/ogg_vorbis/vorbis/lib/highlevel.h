/********************************************************************
 *                                                                  *
 * THIS FILE IS PART OF THE OggVorbis SOFTWARE CODEC SOURCE CODE.   *
 * USE, DISTRIBUTION AND REPRODUCTION OF THIS LIBRARY SOURCE IS     *
 * GOVERNED BY A BSD-STYLE SOURCE LICENSE INCLUDED WITH THIS SOURCE *
 * IN 'COPYING'. PLEASE READ THESE TERMS BEFORE DISTRIBUTING.       *
 *                                                                  *
 * THE OggVorbis SOURCE CODE IS (C) COPYRIGHT 1994-2007             *
 * by the Xiph.Org Foundation http://www.xiph.org/                  *
 *                                                                  *
 ********************************************************************

 function: highlevel encoder setup struct seperated out for vorbisenc clarity
 last mod: $Id: highlevel.h 13293 2007-07-24 00:09:47Z xiphmont $

 ********************************************************************/

typedef struct highlevel_byblocktype {
  float tone_mask_setting;
  float tone_peaklimit_setting;
  float noise_bias_setting;
  float noise_compand_setting;
} highlevel_byblocktype;
  
typedef struct highlevel_encode_setup {
  void *setup;
  int   set_in_stone;

  float base_setting;
  float long_setting;
  float short_setting;
  float impulse_noisetune;

  int    managed;
  ogg_int32_t   bitrate_min;
  ogg_int32_t   bitrate_av;
  float bitrate_av_damp;
  ogg_int32_t   bitrate_max;
  ogg_int32_t   bitrate_reservoir;
  float bitrate_reservoir_bias;
  
  int impulse_block_p;
  int noise_normalize_p;

  float stereo_point_setting;
  float lowpass_kHz;

  float ath_floating_dB;
  float ath_absolute_dB;

  float amplitude_track_dBpersec;
  float trigger_setting;
  
  highlevel_byblocktype block[4]; /* padding, impulse, transition, ogg_int32_t */

} highlevel_encode_setup;

