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

 function: basic shared codebook operations
 last mod: $Id: codebook.h 16227 2009-07-08 06:58:46Z xiphmont $

 ********************************************************************/

#ifndef _V_CODEBOOK_H_
#define _V_CODEBOOK_H_

#include <ogg/ogg.h>

/* This structure encapsulates huffman and VQ style encoding books; it
   doesn't do anything specific to either.

   valuelist/quantlist are nonNULL (and q_* significant) only if
   there's entry->value mapping to be done.

   If encode-side mapping must be done (and thus the entry needs to be
   hunted), the auxiliary encode pointer will point to a decision
   tree.  This is true of both VQ and huffman, but is mostly useful
   with VQ.

*/

typedef struct static_codebook{
  ogg_int32_t   dim;            /* codebook dimensions (elements per vector) */
  ogg_int32_t   entries;        /* codebook entries */
  ogg_int32_t  *lengthlist;     /* codeword lengths in bits */

  /* mapping ***************************************************************/
  int    maptype;        /* 0=none
                            1=implicitly populated values from map column
                            2=listed arbitrary values */

  /* The below does a linear, single monotonic sequence mapping. */
  ogg_int32_t     q_min;       /* packed 32 bit float; quant value 0 maps to minval */
  ogg_int32_t     q_delta;     /* packed 32 bit float; val 1 - val 0 == delta */
  int      q_quant;     /* bits: 0 < quant <= 16 */
  int      q_sequencep; /* bitflag */

  ogg_int32_t     *quantlist;  /* map == 1: (int)(entries^(1/dim)) element column map
                           map == 2: list of dim*entries quantized entry vals
                        */

  /* encode helpers ********************************************************/
//  struct encode_aux_nearestmatch *nearest_tree;
//  struct encode_aux_threshmatch  *thresh_tree;
//  struct encode_aux_pigeonhole  *pigeon_tree;

  int allocedp;
} static_codebook;

/* this structures an arbitrary trained book to quickly find the
   nearest cell match */
//typedef struct encode_aux_nearestmatch{
//  /* pre-calculated partitioning tree */
//  ogg_int32_t   *ptr0;
//  ogg_int32_t   *ptr1;
//
//  ogg_int32_t   *p;         /* decision points (each is an entry) */
//  ogg_int32_t   *q;         /* decision points (each is an entry) */
//  ogg_int32_t   aux;        /* number of tree entries */
//  ogg_int32_t   alloc;
//} encode_aux_nearestmatch;

/* assumes a maptype of 1; encode side only, so that's OK */
//typedef struct encode_aux_threshmatch{
//  float *quantthresh;
//  ogg_int32_t   *quantmap;
//  int     quantvals;
//  int     threshvals;
//} encode_aux_threshmatch;

//typedef struct encode_aux_pigeonhole{
//  float min;
//  float del;
//
//  int  mapentries;
//  int  quantvals;
//  ogg_int32_t *pigeonmap;
//
//  ogg_int32_t fittotal;
//  ogg_int32_t *fitlist;
//  ogg_int32_t *fitmap;
//  ogg_int32_t *fitlength;
//} encode_aux_pigeonhole;

typedef struct codebook{
  ogg_int32_t dim;           /* codebook dimensions (elements per vector) */
  ogg_int32_t entries;       /* codebook entries */
  ogg_int32_t used_entries;  /* populated codebook entries */
  const static_codebook *c;

  /* for encode, the below are entry-ordered, fully populated */
  /* for decode, the below are ordered by bitreversed codeword and only
     used entries are populated */
  float        *valuelist;  /* list of dim*entries actual entry values */
  ogg_uint32_t *codelist;   /* list of bitstream codewords for each entry */

  int          *dec_index;  /* only used if sparseness collapsed */
  char         *dec_codelengths;
  ogg_uint32_t *dec_firsttable;
  int           dec_firsttablen;
  int           dec_maxlength;

} codebook;

extern void FMOD_vorbis_staticbook_clear(static_codebook *b);
extern void FMOD_vorbis_staticbook_destroy(static_codebook *b);
//extern int FMOD_vorbis_book_init_encode(codebook *dest,const static_codebook *source);
extern int FMOD_vorbis_book_init_decode(codebook *dest,const static_codebook *source);
extern void FMOD_vorbis_book_clear(codebook *b);

extern float *_FMOD_book_unquantize(const static_codebook *b,int n,int *map);
extern float *_FMOD_book_logdist(const static_codebook *b,float *vals);
extern float _FMOD_float32_unpack(ogg_int32_t val);
extern ogg_int32_t   _FMOD_float32_pack(float val);
extern int  _FMOD_best(codebook *book, float *a, int step);
extern int _FMOD_ilog(unsigned int v);
extern ogg_int32_t _FMOD_book_maptype1_quantvals(const static_codebook *b);

extern int FMOD_vorbis_book_besterror(codebook *book,float *a,int step,int addmul);
extern ogg_int32_t FMOD_vorbis_book_codeword(codebook *book,int entry);
extern ogg_int32_t FMOD_vorbis_book_codelen(codebook *book,int entry);



extern int FMOD_vorbis_staticbook_pack(const static_codebook *c,oggpack_buffer *b);
extern int FMOD_vorbis_staticbook_unpack(oggpack_buffer *b,static_codebook *c);

//extern int FMOD_vorbis_book_encode(codebook *book, int a, oggpack_buffer *b);
extern int FMOD_vorbis_book_errorv(codebook *book, float *a);
//extern int FMOD_vorbis_book_encodev(codebook *book, int best,float *a,
//                               oggpack_buffer *b);

extern ogg_int32_t FMOD_vorbis_book_decode(codebook *book, oggpack_buffer *b);
extern ogg_int32_t FMOD_vorbis_book_decodevs_add(codebook *book, float *a,
                                     oggpack_buffer *b,int n);
extern ogg_int32_t FMOD_vorbis_book_decodev_set(codebook *book, float *a,
                                    oggpack_buffer *b,int n);
extern ogg_int32_t FMOD_vorbis_book_decodev_add(codebook *book, float *a,
                                    oggpack_buffer *b,int n);
extern ogg_int32_t FMOD_vorbis_book_decodevv_add(codebook *book, float **a,
                                     ogg_int32_t off,int ch,
                                    oggpack_buffer *b,int n);



#endif
