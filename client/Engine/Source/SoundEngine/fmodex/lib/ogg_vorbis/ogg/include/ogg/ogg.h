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

 function: toplevel libogg include
 last mod: $Id: ogg.h 16051 2009-05-27 05:00:06Z xiphmont $

 ********************************************************************/
#ifndef _OGG_H
#define _OGG_H

#ifdef __cplusplus
extern "C" {
#endif

#include <ogg/os_types.h>
#include <stddef.h>

typedef struct {
  void *iov_base;
  size_t iov_len;
} ogg_iovec_t;

typedef struct {
  ogg_int32_t endbyte;
  int  endbit;

  unsigned char *buffer;
  unsigned char *ptr;
  ogg_int32_t storage;
} oggpack_buffer;

/* ogg_page is used to encapsulate the data in one Ogg bitstream page *****/

typedef struct {
  unsigned char *header;
  ogg_int32_t header_len;
  unsigned char *body;
  ogg_int32_t body_len;
} ogg_page;

/* ogg_stream_state contains the current encode/decode state of a logical
   Ogg bitstream **********************************************************/

typedef struct {
  unsigned char   *body_data;    /* bytes from packet bodies */
  ogg_int32_t    body_storage;          /* storage elements allocated */
  ogg_int32_t    body_fill;             /* elements stored; fill mark */
  ogg_int32_t    body_returned;         /* elements of fill returned */


  int     *lacing_vals;      /* The values that will go to the segment table */
  ogg_int64_t *granule_vals; /* granulepos values for headers. Not compact
                                this way, but it is simple coupled to the
                                lacing fifo */
  ogg_int32_t    lacing_storage;
  ogg_int32_t    lacing_fill;
  ogg_int32_t    lacing_packet;
  ogg_int32_t    lacing_returned;

  unsigned char    header[282];      /* working space for header encode */
  int              header_fill;

  int     e_o_s;          /* set when we have buffered the last packet in the
                             logical bitstream */
  int     b_o_s;          /* set after we've written the initial page
                             of a logical bitstream */
  ogg_int32_t    serialno;
  ogg_int32_t    pageno;
  ogg_int64_t  packetno;  /* sequence number for decode; the framing
                             knows where there's a hole in the data,
                             but we need coupling so that the codec
                             (which is in a seperate abstraction
                             layer) also knows about the gap */
  ogg_int64_t   granulepos;

} ogg_stream_state;

/* ogg_packet is used to encapsulate the data and metadata beogg_int32_ting
   to a single raw Ogg/Vorbis packet *************************************/

typedef struct {
  unsigned char *packet;
  ogg_int32_t  bytes;
  ogg_int32_t  b_o_s;
  ogg_int32_t  e_o_s;

  ogg_int64_t  granulepos;

  ogg_int64_t  packetno;     /* sequence number for decode; the framing
                                knows where there's a hole in the data,
                                but we need coupling so that the codec
                                (which is in a seperate abstraction
                                layer) also knows about the gap */
} ogg_packet;

typedef struct {
  unsigned char *data;
  int storage;
  int fill;
  int returned;

  int unsynced;
  int headerbytes;
  int bodybytes;
} ogg_sync_state;

/* Ogg BITSTREAM PRIMITIVES: bitstream ************************/

//extern void  FMOD_oggpack_writeinit(oggpack_buffer *b);
//extern int   FMOD_oggpack_writecheck(oggpack_buffer *b);
//extern void  FMOD_oggpack_writetrunc(oggpack_buffer *b,ogg_int32_t bits);
//extern void  FMOD_oggpack_writealign(oggpack_buffer *b);
//extern void  FMOD_oggpack_writecopy(oggpack_buffer *b,void *source,ogg_int32_t bits);
extern void  FMOD_oggpack_reset(oggpack_buffer *b);
//extern void  FMOD_oggpack_writeclear(oggpack_buffer *b);
extern void  FMOD_oggpack_readinit(oggpack_buffer *b,unsigned char *buf,int bytes);
//extern void  FMOD_oggpack_write(oggpack_buffer *b, ogg_uint32_t value,int bits);
extern ogg_int32_t  FMOD_oggpack_look(oggpack_buffer *b,int bits);
extern ogg_int32_t  FMOD_oggpack_look1(oggpack_buffer *b);
extern void  FMOD_oggpack_adv(oggpack_buffer *b,int bits);
extern void  FMOD_oggpack_adv1(oggpack_buffer *b);
extern ogg_int32_t  FMOD_oggpack_read(oggpack_buffer *b,int bits);
extern ogg_int32_t  FMOD_oggpack_read1(oggpack_buffer *b);
extern ogg_int32_t  FMOD_oggpack_bytes(oggpack_buffer *b);
extern ogg_int32_t  FMOD_oggpack_bits(oggpack_buffer *b);
extern unsigned char *FMOD_oggpack_get_buffer(oggpack_buffer *b);

//extern void  FMOD_oggpackB_writeinit(oggpack_buffer *b);
//extern int   FMOD_oggpackB_writecheck(oggpack_buffer *b);
//extern void  FMOD_oggpackB_writetrunc(oggpack_buffer *b,ogg_int32_t bits);
//extern void  FMOD_oggpackB_writealign(oggpack_buffer *b);
//extern void  FMOD_oggpackB_writecopy(oggpack_buffer *b,void *source,ogg_int32_t bits);
extern void  FMOD_oggpackB_reset(oggpack_buffer *b);
//extern void  FMOD_oggpackB_writeclear(oggpack_buffer *b);
extern void  FMOD_oggpackB_readinit(oggpack_buffer *b,unsigned char *buf,int bytes);
//extern void  FMOD_oggpackB_write(oggpack_buffer *b, ogg_uint32_t value,int bits);
extern ogg_int32_t  FMOD_oggpackB_look(oggpack_buffer *b,int bits);
extern ogg_int32_t  FMOD_oggpackB_look1(oggpack_buffer *b);
extern void  FMOD_oggpackB_adv(oggpack_buffer *b,int bits);
extern void  FMOD_oggpackB_adv1(oggpack_buffer *b);
extern ogg_int32_t  FMOD_oggpackB_read(oggpack_buffer *b,int bits);
extern ogg_int32_t  FMOD_oggpackB_read1(oggpack_buffer *b);
extern ogg_int32_t  FMOD_oggpackB_bytes(oggpack_buffer *b);
extern ogg_int32_t  FMOD_oggpackB_bits(oggpack_buffer *b);
extern unsigned char *FMOD_oggpackB_get_buffer(oggpack_buffer *b);

/* Ogg BITSTREAM PRIMITIVES: encoding **************************/

//extern int      FMOD_ogg_stream_packetin(ogg_stream_state *os, ogg_packet *op);
//extern int      FMOD_ogg_stream_iovecin(ogg_stream_state *os, ogg_iovec_t *iov,
//                                   int count, ogg_int32_t e_o_s, ogg_int64_t granulepos);
//extern int      FMOD_ogg_stream_pageout(ogg_stream_state *os, ogg_page *og);
//extern int      FMOD_ogg_stream_flush(ogg_stream_state *os, ogg_page *og);

/* Ogg BITSTREAM PRIMITIVES: decoding **************************/

extern int      FMOD_ogg_sync_init(ogg_sync_state *oy);
extern int      FMOD_ogg_sync_clear(ogg_sync_state *oy);
extern int      FMOD_ogg_sync_reset(ogg_sync_state *oy);
extern int      FMOD_ogg_sync_destroy(ogg_sync_state *oy);

extern char    *FMOD_ogg_sync_buffer(ogg_sync_state *oy, ogg_int32_t size);
extern int      FMOD_ogg_sync_wrote(ogg_sync_state *oy, ogg_int32_t bytes);
extern ogg_int32_t     FMOD_ogg_sync_pageseek(ogg_sync_state *oy,ogg_page *og);
extern int      FMOD_ogg_sync_pageout(ogg_sync_state *oy, ogg_page *og);
extern int      FMOD_ogg_stream_pagein(ogg_stream_state *os, ogg_page *og);
extern int      FMOD_ogg_stream_packetout(ogg_stream_state *os,ogg_packet *op);
extern int      FMOD_ogg_stream_packetpeek(ogg_stream_state *os,ogg_packet *op);

/* Ogg BITSTREAM PRIMITIVES: general ***************************/

extern int      FMOD_ogg_stream_init(ogg_stream_state *os,int serialno);
extern int      FMOD_ogg_stream_clear(ogg_stream_state *os);
extern int      FMOD_ogg_stream_reset(ogg_stream_state *os);
extern int      FMOD_ogg_stream_reset_serialno(ogg_stream_state *os,int serialno);
extern int      FMOD_ogg_stream_destroy(ogg_stream_state *os);
extern int      FMOD_ogg_stream_eos(ogg_stream_state *os);

extern void     FMOD_ogg_page_checksum_set(ogg_page *og);

extern int      FMOD_ogg_page_version(const ogg_page *og);
extern int      FMOD_ogg_page_continued(const ogg_page *og);
extern int      FMOD_ogg_page_bos(const ogg_page *og);
extern int      FMOD_ogg_page_eos(const ogg_page *og);
extern ogg_int64_t  FMOD_ogg_page_granulepos(const ogg_page *og);
extern int      FMOD_ogg_page_serialno(const ogg_page *og);
extern ogg_int32_t     FMOD_ogg_page_pageno(const ogg_page *og);
extern int      FMOD_ogg_page_packets(const ogg_page *og);

extern void     FMOD_ogg_packet_clear(ogg_packet *op);


#ifdef __cplusplus
}
#endif

#endif  /* _OGG_H */
