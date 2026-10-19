/********************************************************************
 *                                                                  *
 * THIS FILE IS PART OF THE OggVorbis SOFTWARE CODEC SOURCE CODE.   *
 * USE, DISTRIBUTION AND REPRODUCTION OF THIS LIBRARY SOURCE IS     *
 * GOVERNED BY A BSD-STYLE SOURCE LICENSE INCLUDED WITH THIS SOURCE *
 * IN 'COPYING'. PLEASE READ THESE TERMS BEFORE DISTRIBUTING.       *
 *                                                                  *
 * THE OggVorbis SOURCE CODE IS (C) COPYRIGHT 1994-2009             *
 * by the Xiph.Org Foundation http://www.xiph.org/                  *
 *                                                                  *
 ********************************************************************

 function: registry for time, floor, res backends and channel mappings
 last mod: $Id: registry.c 16227 2009-07-08 06:58:46Z xiphmont $

 ********************************************************************/

#include "vorbis/codec.h"
#include "codec_internal.h"
#include "registry.h"
#include "misc.h"
/* seems like major overkill now; the backend numbers will grow into
   the infrastructure soon enough */

extern const vorbis_func_floor     FMOD_floor0_exportbundle;
extern const vorbis_func_floor     FMOD_floor1_exportbundle;
extern const vorbis_func_residue   FMOD_residue0_exportbundle;
extern const vorbis_func_residue   FMOD_residue1_exportbundle;
extern const vorbis_func_residue   FMOD_residue2_exportbundle;
extern const vorbis_func_mapping   FMOD_mapping0_exportbundle;

const vorbis_func_floor     *const _FMOD_floor_P[]={
  &FMOD_floor0_exportbundle,
  &FMOD_floor1_exportbundle,
};

const vorbis_func_residue   *const _FMOD_residue_P[]={
  &FMOD_residue0_exportbundle,
  &FMOD_residue1_exportbundle,
  &FMOD_residue2_exportbundle,
};

const vorbis_func_mapping   *const _FMOD_mapping_P[]={
  &FMOD_mapping0_exportbundle,
};
