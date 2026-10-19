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

  function: packing variable sized words into an octet stream
  last mod: $Id: bitwise.c 16051 2009-05-27 05:00:06Z xiphmont $

 ********************************************************************/

/* We're 'LSb' endian; if we write a word but read individual bits,
   then we'll read the lsb first */

#include <string.h>
#include <stdlib.h>
#include <ogg/ogg.h>

#define BUFFER_INCREMENT 256

static const ogg_uint32_t mask[]=
{0x00000000,0x00000001,0x00000003,0x00000007,0x0000000f,
 0x0000001f,0x0000003f,0x0000007f,0x000000ff,0x000001ff,
 0x000003ff,0x000007ff,0x00000fff,0x00001fff,0x00003fff,
 0x00007fff,0x0000ffff,0x0001ffff,0x0003ffff,0x0007ffff,
 0x000fffff,0x001fffff,0x003fffff,0x007fffff,0x00ffffff,
 0x01ffffff,0x03ffffff,0x07ffffff,0x0fffffff,0x1fffffff,
 0x3fffffff,0x7fffffff,0xffffffff };

void FMOD_oggpack_reset(oggpack_buffer *b){
  if(!b->ptr)return;
  b->ptr=b->buffer;
  b->buffer[0]=0;
  b->endbit=b->endbyte=0;
}

void FMOD_oggpackB_reset(oggpack_buffer *b){
  FMOD_oggpack_reset(b);
}

// VERIFY: 08e54945 auto
// OFFSET: 0x00473240
void FMOD_oggpack_readinit(oggpack_buffer *b,unsigned char *buf,int bytes){
  FMOD_memset(b,0,sizeof(*b));
  b->buffer=b->ptr=buf;
  b->storage=bytes;
}

void FMOD_oggpackB_readinit(oggpack_buffer *b,unsigned char *buf,int bytes){
  FMOD_oggpack_readinit(b,buf,bytes);
}

/* Read in bits without advancing the bitptr; bits <= 32 */
// VERIFY: d970ba23 auto
// OFFSET: 0x00473260
ogg_int32_t FMOD_oggpack_look(oggpack_buffer *b,int bits){
  ogg_uint32_t ret;
  ogg_uint32_t m=mask[bits];

  bits+=b->endbit;

  if(b->endbyte+4>=b->storage){
    /* not the main path */
    if(b->endbyte*8+bits>b->storage*8)return(-1);
  }
  
  ret=b->ptr[0]>>b->endbit;
  if(bits>8){
    ret|=b->ptr[1]<<(8-b->endbit);  
    if(bits>16){
      ret|=b->ptr[2]<<(16-b->endbit);  
      if(bits>24){
        ret|=b->ptr[3]<<(24-b->endbit);  
        if(bits>32 && b->endbit)
          ret|=b->ptr[4]<<(32-b->endbit);
      }
    }
  }
  return(m&ret);
}

/* Read in bits without advancing the bitptr; bits <= 32 */
ogg_int32_t FMOD_oggpackB_look(oggpack_buffer *b,int bits){
  ogg_uint32_t ret;
  int m=32-bits;

  bits+=b->endbit;

  if(b->endbyte+4>=b->storage){
    /* not the main path */
    if(b->endbyte*8+bits>b->storage*8)return(-1);
  }
  
  ret=b->ptr[0]<<(24+b->endbit);
  if(bits>8){
    ret|=b->ptr[1]<<(16+b->endbit);  
    if(bits>16){
      ret|=b->ptr[2]<<(8+b->endbit);  
      if(bits>24){
        ret|=b->ptr[3]<<(b->endbit);  
        if(bits>32 && b->endbit)
          ret|=b->ptr[4]>>(8-b->endbit);
      }
    }
  }
  return ((ret&0xffffffff)>>(m>>1))>>((m+1)>>1);
}

ogg_int32_t FMOD_oggpack_look1(oggpack_buffer *b){
  if(b->endbyte>=b->storage)return(-1);
  return((b->ptr[0]>>b->endbit)&1);
}

ogg_int32_t FMOD_oggpackB_look1(oggpack_buffer *b){
  if(b->endbyte>=b->storage)return(-1);
  return((b->ptr[0]>>(7-b->endbit))&1);
}

// VERIFY: e37b844e auto
// OFFSET: 0x00473310
void FMOD_oggpack_adv(oggpack_buffer *b,int bits){
  bits+=b->endbit;
  b->ptr+=bits/8;
  b->endbyte+=bits/8;
  b->endbit=bits&7;
}

void FMOD_oggpackB_adv(oggpack_buffer *b,int bits){
  FMOD_oggpack_adv(b,bits);
}

void FMOD_oggpack_adv1(oggpack_buffer *b){
  if(++(b->endbit)>7){
    b->endbit=0;
    b->ptr++;
    b->endbyte++;
  }
}

void FMOD_oggpackB_adv1(oggpack_buffer *b){
  FMOD_oggpack_adv1(b);
}

/* bits <= 32 */
// VERIFY: 975a6fa3 auto
// OFFSET: 0x00473340
ogg_int32_t FMOD_oggpack_read(oggpack_buffer *b,int bits){
  ogg_int32_t ret;
  ogg_uint32_t m=mask[bits];

  bits+=b->endbit;

  if(b->endbyte+4>=b->storage){
    /* not the main path */
    ret=-1;
    if(b->endbyte*8+bits>b->storage*8)goto overflow;
  }
  
  ret=b->ptr[0]>>b->endbit;
  if(bits>8){
    ret|=b->ptr[1]<<(8-b->endbit);  
    if(bits>16){
      ret|=b->ptr[2]<<(16-b->endbit);  
      if(bits>24){
        ret|=b->ptr[3]<<(24-b->endbit);  
        if(bits>32 && b->endbit){
          ret|=b->ptr[4]<<(32-b->endbit);
        }
      }
    }
  }
  ret&=m;
  
 overflow:

  b->ptr+=bits/8;
  b->endbyte+=bits/8;
  b->endbit=bits&7;
  return(ret);
}

/* bits <= 32 */
ogg_int32_t FMOD_oggpackB_read(oggpack_buffer *b,int bits){
  ogg_int32_t ret;
  ogg_int32_t m=32-bits;
  
  bits+=b->endbit;

  if(b->endbyte+4>=b->storage){
    /* not the main path */
    ret=-1;
    if(b->endbyte*8+bits>b->storage*8)goto overflow;
    /* special case to avoid reading b->ptr[0], which might be past the end of
        the buffer; also skips some useless accounting */
    else if(!bits)return(0);
  }
  
  ret=b->ptr[0]<<(24+b->endbit);
  if(bits>8){
    ret|=b->ptr[1]<<(16+b->endbit);  
    if(bits>16){
      ret|=b->ptr[2]<<(8+b->endbit);  
      if(bits>24){
        ret|=b->ptr[3]<<(b->endbit);  
        if(bits>32 && b->endbit)
          ret|=b->ptr[4]>>(8-b->endbit);
      }
    }
  }
  ret=((ret&0xffffffff)>>(m>>1))>>((m+1)>>1);
  
 overflow:

  b->ptr+=bits/8;
  b->endbyte+=bits/8;
  b->endbit=bits&7;
  return(ret);
}

ogg_int32_t FMOD_oggpack_read1(oggpack_buffer *b){
  ogg_int32_t ret;
  
  if(b->endbyte>=b->storage){
    /* not the main path */
    ret=-1;
    goto overflow;
  }

  ret=(b->ptr[0]>>b->endbit)&1;
  
 overflow:

  b->endbit++;
  if(b->endbit>7){
    b->endbit=0;
    b->ptr++;
    b->endbyte++;
  }
  return(ret);
}

ogg_int32_t FMOD_oggpackB_read1(oggpack_buffer *b){
  ogg_int32_t ret;
  
  if(b->endbyte>=b->storage){
    /* not the main path */
    ret=-1;
    goto overflow;
  }

  ret=(b->ptr[0]>>(7-b->endbit))&1;
  
 overflow:

  b->endbit++;
  if(b->endbit>7){
    b->endbit=0;
    b->ptr++;
    b->endbyte++;
  }
  return(ret);
}

ogg_int32_t FMOD_oggpack_bytes(oggpack_buffer *b){
  return(b->endbyte+(b->endbit+7)/8);
}

ogg_int32_t FMOD_oggpack_bits(oggpack_buffer *b){
  return(b->endbyte*8+b->endbit);
}

ogg_int32_t FMOD_oggpackB_bytes(oggpack_buffer *b){
  return FMOD_oggpack_bytes(b);
}

ogg_int32_t FMOD_oggpackB_bits(oggpack_buffer *b){
  return FMOD_oggpack_bits(b);
}
  
unsigned char *FMOD_oggpack_get_buffer(oggpack_buffer *b){
  return(b->buffer);
}

unsigned char *FMOD_oggpackB_get_buffer(oggpack_buffer *b){
  return FMOD_oggpack_get_buffer(b);
}

#undef BUFFER_INCREMENT
