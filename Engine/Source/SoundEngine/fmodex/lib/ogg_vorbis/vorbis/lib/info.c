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

 function: maintain the info structure, info <-> header packets
 last mod: $Id: info.c 16243 2009-07-10 02:49:31Z xiphmont $

 ********************************************************************/

/* general handling of the header and the vorbis_info structure (and
   substructures) */

#include <stdlib.h>
#include <string.h>
//#include <ctype.h>
#include <ogg/ogg.h>
#include "vorbis/codec.h"
#include "codec_internal.h"
#include "codebook.h"
#include "registry.h"
#include "window.h"
#include "psy.h"
#include "misc.h"
#include "os.h"

#define GENERAL_VENDOR_STRING "Xiph.Org libVorbis 1.2.3"
#define ENCODE_VENDOR_STRING "Xiph.Org libVorbis I 20090709"

/* helpers */
static int ilog2(unsigned int v){
  int ret=0;
  if(v)--v;
  while(v){
    ret++;
    v>>=1;
  }
  return(ret);
}

char FMOD_ogg_toupper(char in)
{
    if (in >= 'a' && in <= 'z')
    {
        in += 'A' - 'a';
    }

    return in;
}

#if 0
static void _v_writestring(oggpack_buffer *o,const char *s, int bytes){

  while(bytes--){
    FMOD_oggpack_write(o,*s++,8);
  }
}
#endif

// VERIFY: 0dcc538f auto
// OFFSET: 0x0046C090
static void _v_readstring(oggpack_buffer *o,char *buf,int bytes){
  while(bytes--){
    *buf++=(char)FMOD_oggpack_read(o,8);
  }
}

// VERIFY: 9ad0321d auto
// OFFSET: 0x0046C0C0
void FMOD_vorbis_comment_init(vorbis_comment *vc){
  FMOD_memset(vc,0,sizeof(*vc));
}

//void FMOD_vorbis_comment_add(vorbis_comment *vc,const char *comment){
//  vc->user_comments=_ogg_realloc(vc->user_comments,
//                            (vc->comments+2)*sizeof(*vc->user_comments));
//  vc->comment_lengths=_ogg_realloc(vc->comment_lengths,
//                                  (vc->comments+2)*sizeof(*vc->comment_lengths));
//  vc->comment_lengths[vc->comments]=ogg_strlen(comment);
//  vc->user_comments[vc->comments]=_ogg_malloc(vc->comment_lengths[vc->comments]+1);
//  ogg_strcpy(vc->user_comments[vc->comments], comment);
//  vc->comments++;
//  vc->user_comments[vc->comments]=NULL;
//}
//
//void vorbis_comment_add_tag(vorbis_comment *vc, const char *tag, const char *contents){
//  char *comment=alloca(ogg_strlen(tag)+ogg_strlen(contents)+2); /* +2 for = and \0 */
//  ogg_strcpy(comment, tag);
//  ogg_strcat(comment, "=");
//  ogg_strcat(comment, contents);
//  FMOD_vorbis_comment_add(vc, comment);
//}

/* This is more or less the same as strncasecmp - but that doesn't exist
 * everywhere, and this is a fairly trivial function, so we include it */
static int tagcompare(const char *s1, const char *s2, int n){
  int c=0;
  while(c < n){
    if(FMOD_ogg_toupper(s1[c]) != FMOD_ogg_toupper(s2[c]))
      return !0;
    c++;
  }
  return 0;
}

//char *FMOD_vorbis_comment_query(vorbis_comment *vc, const char *tag, int count){
//  ogg_int32_t i;
//  int found = 0;
//  int taglen = ogg_strlen(tag)+1; /* +1 for the = we append */
//  char *fulltag = alloca(taglen+ 1);
//
//  ogg_strcpy(fulltag, tag);
//  ogg_strcat(fulltag, "=");
//
//  for(i=0;i<vc->comments;i++){
//    if(!tagcompare(vc->user_comments[i], fulltag, taglen)){
//      if(count == found)
//        /* We return a pointer to the data, not a copy */
//              return vc->user_comments[i] + taglen;
//      else
//        found++;
//    }
//  }
//  return NULL; /* didn't find anything */
//}
//
//int FMOD_vorbis_comment_query_count(vorbis_comment *vc, const char *tag){
//  int i,count=0;
//  int taglen = ogg_strlen(tag)+1; /* +1 for the = we append */
//  char *fulltag = alloca(taglen+1);
//  ogg_strcpy(fulltag,tag);
//  ogg_strcat(fulltag, "=");
//
//  for(i=0;i<vc->comments;i++){
//    if(!tagcompare(vc->user_comments[i], fulltag, taglen))
//      count++;
//  }
//
//  return count;
//}

// VERIFY: 79a6fb5f auto
// OFFSET: 0x0046C0E0
void FMOD_vorbis_comment_clear(vorbis_comment *vc){
  if(vc){
    ogg_int32_t i;
    for(i=0;i<vc->comments;i++)
      if(vc->user_comments[i])_ogg_free(vc->user_comments[i]);
    if(vc->user_comments)_ogg_free(vc->user_comments);
    if(vc->comment_lengths)_ogg_free(vc->comment_lengths);
    if(vc->vendor)_ogg_free(vc->vendor);
    FMOD_memset(vc,0,sizeof(*vc));
  }
}

/* blocksize 0 is guaranteed to be short, 1 is guaranteed to be long.
   They may be equal, but short will never ge greater than long */
// VERIFY: 232d2834 auto
// OFFSET: 0x0046C180
int FMOD_vorbis_info_blocksize(vorbis_info *vi,int zo){
  codec_setup_info *ci = vi->codec_setup;
  return ci ? ci->blocksizes[zo] : -1;
}

/* used by synthesis, which has a full, alloced vi */
// VERIFY: d23f2fb0 auto
// OFFSET: 0x0046C1A0
int FMOD_vorbis_info_init(vorbis_info *vi){
  FMOD_memset(vi,0,sizeof(*vi));
  vi->codec_setup=_ogg_calloc(1,sizeof(codec_setup_info));
  if(!vi->codec_setup)
  {
    return(OV_EMEMORY);
  }

  return 0;
}

// VERIFY: e19c8e0f manual
// OFFSET: 0x0046C1F0
void FMOD_vorbis_info_clear(vorbis_info *vi){
  codec_setup_info     *ci=vi->codec_setup;
  int i;

  if(ci){

    for(i=0;i<ci->modes;i++)
      if(ci->mode_param[i])_ogg_free(ci->mode_param[i]);

    for(i=0;i<ci->maps;i++) /* unpack does the range checking */
      if(ci->map_param[i]) /* this may be cleaning up an aborted
                              unpack, in which case the below type
                              cannot be trusted */
        _FMOD_mapping_P[ci->map_type[i]]->free_info(ci->map_param[i]);

    for(i=0;i<ci->floors;i++) /* unpack does the range checking */
      if(ci->floor_param[i]) /* this may be cleaning up an aborted
                                unpack, in which case the below type
                                cannot be trusted */
        _FMOD_floor_P[ci->floor_type[i]]->free_info(ci->floor_param[i]);

    for(i=0;i<ci->residues;i++) /* unpack does the range checking */
      if(ci->residue_param[i]) /* this may be cleaning up an aborted
                                  unpack, in which case the below type
                                  cannot be trusted */
        _FMOD_residue_P[ci->residue_type[i]]->free_info(ci->residue_param[i]);

    for(i=0;i<ci->books;i++){
      if(ci->book_param[i]){
        /* knows if the book was not alloced */
        FMOD_vorbis_staticbook_destroy(ci->book_param[i]);
      }
      if(ci->fullbooks)
        FMOD_vorbis_book_clear(ci->fullbooks+i);
    }
    if(ci->fullbooks)
        _ogg_free(ci->fullbooks);

    for(i=0;i<ci->psys;i++)
      _vi_psy_free(ci->psy_param[i]);

    _ogg_free(ci);
  }

  FMOD_memset(vi,0,sizeof(*vi));
}

/* Header packing/unpacking ********************************************/

// VERIFY: 2ca50442 auto
// OFFSET: 0x0046C3A0
static int _FMOD_vorbis_unpack_info(vorbis_info *vi,oggpack_buffer *opb){
  codec_setup_info     *ci=vi->codec_setup;
  if(!ci)return(OV_EFAULT);

  vi->version=FMOD_oggpack_read(opb,32);
  if(vi->version!=0)return(OV_EVERSION);

  vi->channels=FMOD_oggpack_read(opb,8);
  vi->rate=FMOD_oggpack_read(opb,32);

  vi->bitrate_upper=FMOD_oggpack_read(opb,32);
  vi->bitrate_nominal=FMOD_oggpack_read(opb,32);
  vi->bitrate_lower=FMOD_oggpack_read(opb,32);

  ci->blocksizes[0]=1<<FMOD_oggpack_read(opb,4);
  ci->blocksizes[1]=1<<FMOD_oggpack_read(opb,4);

  if(vi->rate<1)goto err_out;
  if(vi->channels<1)goto err_out;
  if(ci->blocksizes[0]<64)goto err_out;
  if(ci->blocksizes[1]<ci->blocksizes[0])goto err_out;
  if(ci->blocksizes[1]>8192)goto err_out;

  if(FMOD_oggpack_read(opb,1)!=1)goto err_out; /* EOP check */

  return(0);
 err_out:
  FMOD_vorbis_info_clear(vi);
  return(OV_EBADHEADER);
}

// VERIFY: 09994433 manual
// OFFSET: 0x0046C470
static int _vorbis_unpack_comment(vorbis_comment *vc,oggpack_buffer *opb){
  int i;
  int vendorlen=FMOD_oggpack_read(opb,32);
  if(vendorlen<0)goto err_out;
  vc->vendor=_ogg_calloc(vendorlen+1,1);
  if(!vc->vendor)goto err_out;
  _v_readstring(opb,vc->vendor,vendorlen);
  vc->comments=FMOD_oggpack_read(opb,32);
  if(vc->comments<0)goto err_out;
  vc->user_comments=_ogg_calloc(vc->comments+1,sizeof(*vc->user_comments));
  if(!vc->user_comments)goto err_out;
  vc->comment_lengths=_ogg_calloc(vc->comments+1, sizeof(*vc->comment_lengths));
  if(!vc->comment_lengths)goto err_out;

  for(i=0;i<vc->comments;i++){
    int len=FMOD_oggpack_read(opb,32);
    if(len<0)goto err_out;
    vc->comment_lengths[i]=len;
    vc->user_comments[i]=_ogg_calloc(len+1,1);
    if(!vc->user_comments)goto err_out;
    _v_readstring(opb,vc->user_comments[i],len);
  }
  if(FMOD_oggpack_read(opb,1)!=1)goto err_out; /* EOP check */

  return(0);
 err_out:
  FMOD_vorbis_comment_clear(vc);
  return(OV_EBADHEADER);
}

/* all of the real encoding details are here.  The modes, books,
   everything */
// VERIFY: 9a9e21d4 manual
// OFFSET: 0x0046C5A0
static int _vorbis_unpack_books(vorbis_info *vi,oggpack_buffer *opb){
  codec_setup_info     *ci=vi->codec_setup;
  int i;
  if(!ci)return(OV_EFAULT);

  /* codebooks */
  ci->books=FMOD_oggpack_read(opb,8)+1;
  for(i=0;i<ci->books;i++){
    ci->book_param[i]=_ogg_calloc(1,sizeof(*ci->book_param[i]));
    if(!ci->book_param[i])goto err_out;
    if(FMOD_vorbis_staticbook_unpack(opb,ci->book_param[i]))goto err_out;
  }

  /* time backend settings; hooks are unused */
  {
    int times=FMOD_oggpack_read(opb,6)+1;
    for(i=0;i<times;i++){
      int test=FMOD_oggpack_read(opb,16);
      if(test<0 || test>=VI_TIMEB)goto err_out;
    }
  }

  /* floor backend settings */
  ci->floors=FMOD_oggpack_read(opb,6)+1;
  for(i=0;i<ci->floors;i++){
    ci->floor_type[i]=FMOD_oggpack_read(opb,16);
    if(ci->floor_type[i]<0 || ci->floor_type[i]>=VI_FLOORB)goto err_out;
    ci->floor_param[i]=_FMOD_floor_P[ci->floor_type[i]]->unpack(vi,opb);
    if(!ci->floor_param[i])goto err_out;
  }

  /* residue backend settings */
  ci->residues=FMOD_oggpack_read(opb,6)+1;
  for(i=0;i<ci->residues;i++){
    ci->residue_type[i]=FMOD_oggpack_read(opb,16);
    if(ci->residue_type[i]<0 || ci->residue_type[i]>=VI_RESB)goto err_out;
    ci->residue_param[i]=_FMOD_residue_P[ci->residue_type[i]]->unpack(vi,opb);
    if(!ci->residue_param[i])goto err_out;
  }

  /* map backend settings */
  ci->maps=FMOD_oggpack_read(opb,6)+1;
  for(i=0;i<ci->maps;i++){
    ci->map_type[i]=FMOD_oggpack_read(opb,16);
    if(ci->map_type[i]<0 || ci->map_type[i]>=VI_MAPB)goto err_out;
    ci->map_param[i]=_FMOD_mapping_P[ci->map_type[i]]->unpack(vi,opb);
    if(!ci->map_param[i])goto err_out;
  }

  /* mode settings */
  ci->modes=FMOD_oggpack_read(opb,6)+1;
  for(i=0;i<ci->modes;i++){
    ci->mode_param[i]=_ogg_calloc(1,sizeof(*ci->mode_param[i]));
    if(!ci->mode_param[i])goto err_out;
    ci->mode_param[i]->blockflag=FMOD_oggpack_read(opb,1);
    ci->mode_param[i]->windowtype=FMOD_oggpack_read(opb,16);
    ci->mode_param[i]->transformtype=FMOD_oggpack_read(opb,16);
    ci->mode_param[i]->mapping=FMOD_oggpack_read(opb,8);

    if(ci->mode_param[i]->windowtype>=VI_WINDOWB)goto err_out;
    if(ci->mode_param[i]->transformtype>=VI_WINDOWB)goto err_out;
    if(ci->mode_param[i]->mapping>=ci->maps)goto err_out;
  }

  if(FMOD_oggpack_read(opb,1)!=1)goto err_out; /* top level EOP check */

  return(0);
 err_out:
  FMOD_vorbis_info_clear(vi);
  return(OV_EBADHEADER);
}

/* Is this packet a vorbis ID header? */
// VERIFY: 1dbc863f manual
// OFFSET: 0x0046C890
int FMOD_vorbis_synthesis_idheader(ogg_packet *op){
  oggpack_buffer opb;
  char buffer[6];

  if(op){
    FMOD_oggpack_readinit(&opb,op->packet,op->bytes);

    if(!op->b_o_s)
      return(0); /* Not the initial packet */

    if(FMOD_oggpack_read(&opb,8) != 1)
      return 0; /* not an ID header */

    FMOD_memset(buffer,0,6);
    _v_readstring(&opb,buffer,6);
    if(memcmp(buffer,"vorbis",6))
      return 0; /* not vorbis */

    return 1;
  }

  return 0;
}

/* The Vorbis header is in three packets; the initial small packet in
   the first page that identifies basic parameters, a second packet
   with bitstream comments and a third packet that holds the
   codebook. */

// VERIFY: a973022c manual
// OFFSET: 0x0046C9B0
int FMOD_vorbis_synthesis_headerin(vorbis_info *vi,vorbis_comment *vc,ogg_packet *op){
  oggpack_buffer opb;

  if(op){
    FMOD_oggpack_readinit(&opb,op->packet,op->bytes);

    /* Which of the three types of header is this? */
    /* Also verify header-ness, vorbis */
    {
      char buffer[6];
      int packtype=FMOD_oggpack_read(&opb,8);
      FMOD_memset(buffer,0,6);
      _v_readstring(&opb,buffer,6);
      if(memcmp(buffer,"vorbis",6)){
        /* not a vorbis header */
        return(OV_ENOTVORBIS);
      }
      switch(packtype){
      case 0x01: /* least significant *bit* is read first */
        if(!op->b_o_s){
          /* Not the initial packet */
          return(OV_EBADHEADER);
        }
        if(vi->rate!=0){
          /* previously initialized info header */
          return(OV_EBADHEADER);
        }

        return(_FMOD_vorbis_unpack_info(vi,&opb));

      case 0x03: /* least significant *bit* is read first */
        if(vi->rate==0){
          /* um... we didn't get the initial header */
          return(OV_EBADHEADER);
        }

        return(_vorbis_unpack_comment(vc,&opb));

      case 0x05: /* least significant *bit* is read first */
        if(vi->rate==0 || vc->vendor==NULL){
          /* um... we didn;t get the initial header or comments yet */
          return(OV_EBADHEADER);
        }

        return(_vorbis_unpack_books(vi,&opb));

      default:
        /* Not a valid vorbis header type */
        return(OV_EBADHEADER);
        break;
      }
    }
  }
  return(OV_EBADHEADER);
}

///* pack side **********************************************************/
//
//static int _vorbis_pack_info(oggpack_buffer *opb,vorbis_info *vi){
//  codec_setup_info     *ci=vi->codec_setup;
//  if(!ci)return(OV_EFAULT);
//
//  /* preamble */
//  FMOD_oggpack_write(opb,0x01,8);
//  _v_writestring(opb,"vorbis", 6);
//
//  /* basic information about the stream */
//  FMOD_oggpack_write(opb,0x00,32);
//  FMOD_oggpack_write(opb,vi->channels,8);
//  FMOD_oggpack_write(opb,vi->rate,32);
//
//  FMOD_oggpack_write(opb,vi->bitrate_upper,32);
//  FMOD_oggpack_write(opb,vi->bitrate_nominal,32);
//  FMOD_oggpack_write(opb,vi->bitrate_lower,32);
//
//  FMOD_oggpack_write(opb,ilog2(ci->blocksizes[0]),4);
//  FMOD_oggpack_write(opb,ilog2(ci->blocksizes[1]),4);
//  FMOD_oggpack_write(opb,1,1);
//
//  return(0);
//}
//
//static int _vorbis_pack_comment(oggpack_buffer *opb,vorbis_comment *vc){
//  int bytes = ogg_strlen(ENCODE_VENDOR_STRING);
//
//  /* preamble */
//  FMOD_oggpack_write(opb,0x03,8);
//  _v_writestring(opb,"vorbis", 6);
//
//  /* vendor */
//  FMOD_oggpack_write(opb,bytes,32);
//  _v_writestring(opb,ENCODE_VENDOR_STRING, bytes);
//
//  /* comments */
//
//  FMOD_oggpack_write(opb,vc->comments,32);
//  if(vc->comments){
//    int i;
//    for(i=0;i<vc->comments;i++){
//      if(vc->user_comments[i]){
//        FMOD_oggpack_write(opb,vc->comment_lengths[i],32);
//        _v_writestring(opb,vc->user_comments[i], vc->comment_lengths[i]);
//      }else{
//        FMOD_oggpack_write(opb,0,32);
//      }
//    }
//  }
//  FMOD_oggpack_write(opb,1,1);
//
//  return(0);
//}
//
//static int _vorbis_pack_books(oggpack_buffer *opb,vorbis_info *vi){
//  codec_setup_info     *ci=vi->codec_setup;
//  int i;
//  if(!ci)return(OV_EFAULT);
//
//  FMOD_oggpack_write(opb,0x05,8);
//  _v_writestring(opb,"vorbis", 6);
//
//  /* books */
//  FMOD_oggpack_write(opb,ci->books-1,8);
//  for(i=0;i<ci->books;i++)
//    if(FMOD_vorbis_staticbook_pack(ci->book_param[i],opb))goto err_out;
//
//  /* times; hook placeholders */
//  FMOD_oggpack_write(opb,0,6);
//  FMOD_oggpack_write(opb,0,16);
//
//  /* floors */
//  FMOD_oggpack_write(opb,ci->floors-1,6);
//  for(i=0;i<ci->floors;i++){
//    FMOD_oggpack_write(opb,ci->floor_type[i],16);
//    if(_floor_P[ci->floor_type[i]]->pack)
//      _floor_P[ci->floor_type[i]]->pack(ci->floor_param[i],opb);
//    else
//      goto err_out;
//  }
//
//  /* residues */
//  FMOD_oggpack_write(opb,ci->residues-1,6);
//  for(i=0;i<ci->residues;i++){
//    FMOD_oggpack_write(opb,ci->residue_type[i],16);
//    _FMOD_residue_P[ci->residue_type[i]]->pack(ci->residue_param[i],opb);
//  }
//
//  /* maps */
//  FMOD_oggpack_write(opb,ci->maps-1,6);
//  for(i=0;i<ci->maps;i++){
//    FMOD_oggpack_write(opb,ci->map_type[i],16);
//    _FMOD_mapping_P[ci->map_type[i]]->pack(vi,ci->map_param[i],opb);
//  }
//
//  /* modes */
//  FMOD_oggpack_write(opb,ci->modes-1,6);
//  for(i=0;i<ci->modes;i++){
//    FMOD_oggpack_write(opb,ci->mode_param[i]->blockflag,1);
//    FMOD_oggpack_write(opb,ci->mode_param[i]->windowtype,16);
//    FMOD_oggpack_write(opb,ci->mode_param[i]->transformtype,16);
//    FMOD_oggpack_write(opb,ci->mode_param[i]->mapping,8);
//  }
//  FMOD_oggpack_write(opb,1,1);
//
//  return(0);
//err_out:
//  return(-1);
//}
//
//int vorbis_commentheader_out(vorbis_comment *vc,
//                                          ogg_packet *op){
//
//  oggpack_buffer opb;
//
//  FMOD_oggpack_writeinit(&opb);
//  if(_vorbis_pack_comment(&opb,vc)) return OV_EIMPL;
//
//  op->packet = _ogg_malloc(FMOD_oggpack_bytes(&opb));
//  FMOD_memcpy(op->packet, opb.buffer, FMOD_oggpack_bytes(&opb));
//
//  op->bytes=FMOD_oggpack_bytes(&opb);
//  op->b_o_s=0;
//  op->e_o_s=0;
//  op->granulepos=0;
//  op->packetno=1;
//
//  return 0;
//}
//
//int vorbis_analysis_headerout(vorbis_dsp_state *v,
//                              vorbis_comment *vc,
//                              ogg_packet *op,
//                              ogg_packet *op_comm,
//                              ogg_packet *op_code){
//  int ret=OV_EIMPL;
//  vorbis_info *vi=v->vi;
//  oggpack_buffer opb;
//  private_state *b=v->backend_state;
//
//  if(!b){
//    ret=OV_EFAULT;
//    goto err_out;
//  }
//
//  /* first header packet **********************************************/
//
//  FMOD_oggpack_writeinit(&opb);
//  if(_vorbis_pack_info(&opb,vi))goto err_out;
//
//  /* build the packet */
//  if(b->header)_ogg_free(b->header);
//  b->header=_ogg_malloc(FMOD_oggpack_bytes(&opb));
//  FMOD_memcpy(b->header,opb.buffer,FMOD_oggpack_bytes(&opb));
//  op->packet=b->header;
//  op->bytes=FMOD_oggpack_bytes(&opb);
//  op->b_o_s=1;
//  op->e_o_s=0;
//  op->granulepos=0;
//  op->packetno=0;
//
//  /* second header packet (comments) **********************************/
//
//  oggpack_reset(&opb);
//  if(_vorbis_pack_comment(&opb,vc))goto err_out;
//
//  if(b->header1)_ogg_free(b->header1);
//  b->header1=_ogg_malloc(FMOD_oggpack_bytes(&opb));
//  FMOD_memcpy(b->header1,opb.buffer,FMOD_oggpack_bytes(&opb));
//  op_comm->packet=b->header1;
//  op_comm->bytes=FMOD_oggpack_bytes(&opb);
//  op_comm->b_o_s=0;
//  op_comm->e_o_s=0;
//  op_comm->granulepos=0;
//  op_comm->packetno=1;
//
//  /* third header packet (modes/codebooks) ****************************/
//
//  oggpack_reset(&opb);
//  if(_vorbis_pack_books(&opb,vi))goto err_out;
//
//  if(b->header2)_ogg_free(b->header2);
//  b->header2=_ogg_malloc(FMOD_oggpack_bytes(&opb));
//  FMOD_memcpy(b->header2,opb.buffer,FMOD_oggpack_bytes(&opb));
//  op_code->packet=b->header2;
//  op_code->bytes=FMOD_oggpack_bytes(&opb);
//  op_code->b_o_s=0;
//  op_code->e_o_s=0;
//  op_code->granulepos=0;
//  op_code->packetno=2;
//
//  FMOD_oggpack_writeclear(&opb);
//  return(0);
// err_out:
//  FMOD_oggpack_writeclear(&opb);
//  FMOD_memset(op,0,sizeof(*op));
//  FMOD_memset(op_comm,0,sizeof(*op_comm));
//  FMOD_memset(op_code,0,sizeof(*op_code));
//
//  if(b){
//    if(b->header)_ogg_free(b->header);
//    if(b->header1)_ogg_free(b->header1);
//    if(b->header2)_ogg_free(b->header2);
//    b->header=NULL;
//    b->header1=NULL;
//    b->header2=NULL;
//  }
//  return(ret);
//}
//
//double vorbis_granule_time(vorbis_dsp_state *v,ogg_int64_t granulepos){
//  if(granulepos>=0)
//    return((double)granulepos/v->vi->rate);
//  return(-1);
//}
//
//const char *vorbis_version_string(void){
//  return GENERAL_VENDOR_STRING;
//}
