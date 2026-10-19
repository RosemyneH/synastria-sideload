/*
 * jcmarker.c
 *
 * Copyright (C) 1991-1998, Thomas G. Lane.
 * This file is part of the Independent JPEG Group's software.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This file contains routines to write JPEG datastream markers.
 */

#define JPEG_INTERNALS
#include "jinclude.h"
#include "jpeglib.h"


typedef enum {			/* JPEG marker codes */
  M_SOF0  = 0xc0,
  M_SOF1  = 0xc1,
  M_SOF2  = 0xc2,
  M_SOF3  = 0xc3,

  M_SOF5  = 0xc5,
  M_SOF6  = 0xc6,
  M_SOF7  = 0xc7,

  M_JPG   = 0xc8,
  M_SOF9  = 0xc9,
  M_SOF10 = 0xca,
  M_SOF11 = 0xcb,

  M_SOF13 = 0xcd,
  M_SOF14 = 0xce,
  M_SOF15 = 0xcf,

  M_DHT   = 0xc4,

  M_DAC   = 0xcc,

  M_RST0  = 0xd0,
  M_RST1  = 0xd1,
  M_RST2  = 0xd2,
  M_RST3  = 0xd3,
  M_RST4  = 0xd4,
  M_RST5  = 0xd5,
  M_RST6  = 0xd6,
  M_RST7  = 0xd7,

  M_SOI   = 0xd8,
  M_EOI   = 0xd9,
  M_SOS   = 0xda,
  M_DQT   = 0xdb,
  M_DNL   = 0xdc,
  M_DRI   = 0xdd,
  M_DHP   = 0xde,
  M_EXP   = 0xdf,

  M_APP0  = 0xe0,
  M_APP1  = 0xe1,
  M_APP2  = 0xe2,
  M_APP3  = 0xe3,
  M_APP4  = 0xe4,
  M_APP5  = 0xe5,
  M_APP6  = 0xe6,
  M_APP7  = 0xe7,
  M_APP8  = 0xe8,
  M_APP9  = 0xe9,
  M_APP10 = 0xea,
  M_APP11 = 0xeb,
  M_APP12 = 0xec,
  M_APP13 = 0xed,
  M_APP14 = 0xee,
  M_APP15 = 0xef,

  M_JPG0  = 0xf0,
  M_JPG13 = 0xfd,
  M_COM   = 0xfe,

  M_TEM   = 0x01,

  M_ERROR = 0x100
} JPEG_MARKER;


/* Private state */

typedef struct {
  struct jpeg_marker_writer pub; /* public fields */

  unsigned int last_restart_interval; /* last DRI value emitted; 0 after SOI */
} my_marker_writer;

typedef my_marker_writer * my_marker_ptr;


/*
 * Basic output routines.
 *
 * Note that we do not support suspension while writing a marker.
 * Therefore, an application using suspension must ensure that there is
 * enough buffer space for the initial markers (typ. 600-700 bytes) before
 * calling jpeg_start_compress, and enough space to write the trailing EOI
 * (a few bytes) before calling jpeg_finish_compress.  Multipass compression
 * modes are not supported at all with suspension, so those two are the only
 * points where markers will be written.
 */

/* Emit a byte */
// VERIFY: aed36e97 auto
// OFFSET: 0x004ACF00
static void emit_byte(j_compress_ptr cinfo, int val) {
  struct jpeg_destination_mgr * dest = cinfo->dest;

  *(dest->next_output_byte)++ = (JOCTET) val;
  if (--dest->free_in_buffer == 0) {
    if (! (*dest->empty_output_buffer) (cinfo))
      ERREXIT(cinfo, JERR_CANT_SUSPEND);
  }
}


/* Emit a marker code */
// VERIFY: 72610808 auto
// OFFSET: 0x004ACF40
static void emit_marker(j_compress_ptr cinfo, JPEG_MARKER mark) {
  emit_byte(cinfo, 0xFF);
  if (cinfo->error_flag)
    return;
  emit_byte(cinfo, (int) mark);
}


/* Emit a 2-byte integer; these are always MSB first in JPEG files */
// VERIFY: 474a813f auto
// OFFSET: 0x004ACFB0
static void emit_2bytes(j_compress_ptr cinfo, int value) {
  emit_byte(cinfo, (value >> 8) & 0xFF);
  if (cinfo->error_flag)
    return;
  emit_byte(cinfo, value & 0xFF);
}


/*
 * Routines to write specific marker types.
 */

/* Emit a DQT marker */
/* Returns the precision used (0 = 8bits, 1 = 16bits) for baseline checking */
// VERIFY: 68038cb8 auto
// OFFSET: 0x004AD020
static int emit_dqt(j_compress_ptr cinfo, int index) {
  JQUANT_TBL * qtbl = cinfo->quant_tbl_ptrs[index];
  int prec;
  int i;

  if (qtbl == NULL) {
    ERREXIT1(cinfo, JERR_NO_QUANT_TABLE, index);
    return 0;
  }

  prec = 0;
  for (i = 0; i < DCTSIZE2; i++) {
    if (qtbl->quantval[i] > 255)
      prec = 1;
  }

  if (! qtbl->sent_table) {
    emit_marker(cinfo, M_DQT);
    if (cinfo->error_flag)
      return 0;

    emit_2bytes(cinfo, prec ? DCTSIZE2*2 + 1 + 2 : DCTSIZE2 + 1 + 2);
    if (cinfo->error_flag)
      return 0;

    emit_byte(cinfo, index + (prec<<4));
    if (cinfo->error_flag)
      return 0;

    for (i = 0; i < DCTSIZE2; i++) {
      /* The table entries must be emitted in zigzag order. */
      unsigned int qval = qtbl->quantval[jpeg_natural_order[i]];
      if (prec) {
	emit_byte(cinfo, (int) (qval >> 8));
	if (cinfo->error_flag)
	  return 0;
      }
      emit_byte(cinfo, (int) (qval & 0xFF));
      if (cinfo->error_flag)
        return 0;
    }

    qtbl->sent_table = TRUE;
  }

  return prec;
}


/* Emit a DHT marker */
// VERIFY: 6dab3d45 manual
// OFFSET: 0x004AD1E0
static void emit_dht(j_compress_ptr cinfo, int index, boolean is_ac) {
  JHUFF_TBL * htbl;
  int length, i;

  if (is_ac) {
    htbl = cinfo->ac_huff_tbl_ptrs[index];
    index += 0x10;		/* output index has AC bit set */
  } else {
    htbl = cinfo->dc_huff_tbl_ptrs[index];
  }

  if (htbl == NULL) {
    ERREXIT1(cinfo, JERR_NO_HUFF_TABLE, index);
    return;
  }

  if (! htbl->sent_table) {
    emit_marker(cinfo, M_DHT);
    if (cinfo->error_flag)
      return;

    length = 0;
    for (i = 1; i <= 16; i++)
      length += htbl->bits[i];

    emit_2bytes(cinfo, length + 2 + 1 + 16);
    if (cinfo->error_flag)
      return;
    emit_byte(cinfo, index);
    if (cinfo->error_flag)
      return;

    for (i = 1; i <= 16; i++) {
      emit_byte(cinfo, htbl->bits[i]);
      if (cinfo->error_flag)
        return;
    }

    for (i = 0; i < length; i++) {
      emit_byte(cinfo, htbl->huffval[i]);
      if (cinfo->error_flag)
        return;
    }

    htbl->sent_table = TRUE;
  }
}


/* Emit a DAC marker */
/* Since the useful info is so small, we want to emit all the tables in */
/* one DAC marker.  Therefore this routine does its own scan of the table. */
static void emit_dac(j_compress_ptr cinfo) {
#ifdef C_ARITH_CODING_SUPPORTED
  char dc_in_use[NUM_ARITH_TBLS];
  char ac_in_use[NUM_ARITH_TBLS];
  int length, i;
  jpeg_component_info *compptr;

  for (i = 0; i < NUM_ARITH_TBLS; i++)
    dc_in_use[i] = ac_in_use[i] = 0;

  for (i = 0; i < cinfo->comps_in_scan; i++) {
    compptr = cinfo->cur_comp_info[i];
    dc_in_use[compptr->dc_tbl_no] = 1;
    ac_in_use[compptr->ac_tbl_no] = 1;
  }

  length = 0;
  for (i = 0; i < NUM_ARITH_TBLS; i++)
    length += dc_in_use[i] + ac_in_use[i];

  emit_marker(cinfo, M_DAC);

  emit_2bytes(cinfo, length*2 + 2);

  for (i = 0; i < NUM_ARITH_TBLS; i++) {
    if (dc_in_use[i]) {
      emit_byte(cinfo, i);
      emit_byte(cinfo, cinfo->arith_dc_L[i] + (cinfo->arith_dc_U[i]<<4));
    }
    if (ac_in_use[i]) {
      emit_byte(cinfo, i + 0x10);
      emit_byte(cinfo, cinfo->arith_ac_K[i]);
    }
  }
#endif /* C_ARITH_CODING_SUPPORTED */
}


/* Emit a DRI marker */
static void emit_dri(j_compress_ptr cinfo) {
  emit_marker(cinfo, M_DRI);
  if (cinfo->error_flag)
    return;

  emit_2bytes(cinfo, 4);	/* fixed length */
  if (cinfo->error_flag)
    return;

  emit_2bytes(cinfo, (int) cinfo->restart_interval);
}


/* Emit a SOF marker */
// VERIFY: 920c6e92 auto
// OFFSET: 0x004AD380
static void emit_sof(j_compress_ptr cinfo, JPEG_MARKER code) {
  int ci;
  jpeg_component_info *compptr;

  emit_marker(cinfo, code);
  if (cinfo->error_flag)
    return;

  emit_2bytes(cinfo, 3 * cinfo->num_components + 2 + 5 + 1); /* length */
  if (cinfo->error_flag)
    return;

  /* Make sure image isn't bigger than SOF field can handle */
  if ((long) cinfo->image_height > 65535L ||
      (long) cinfo->image_width > 65535L) {
    ERREXIT1(cinfo, JERR_IMAGE_TOO_BIG, (unsigned int) 65535);
    return;
  }

  emit_byte(cinfo, cinfo->data_precision);
  if (cinfo->error_flag)
    return;
  emit_2bytes(cinfo, (int) cinfo->image_height);
  if (cinfo->error_flag)
    return;
  emit_2bytes(cinfo, (int) cinfo->image_width);
  if (cinfo->error_flag)
    return;

  emit_byte(cinfo, cinfo->num_components);
  if (cinfo->error_flag)
    return;

  for (ci = 0, compptr = cinfo->comp_info; ci < cinfo->num_components;
       ci++, compptr++) {
    emit_byte(cinfo, compptr->component_id);
    if (cinfo->error_flag)
      return;
    emit_byte(cinfo, (compptr->h_samp_factor << 4) + compptr->v_samp_factor);
    if (cinfo->error_flag)
      return;
    emit_byte(cinfo, compptr->quant_tbl_no);
    if (cinfo->error_flag)
      return;
  }
}


/* Emit a SOS marker */
// VERIFY: c8f0f468 manual
// OFFSET: 0x004AD480
static void emit_sos(j_compress_ptr cinfo) {
  int i, td, ta;
  jpeg_component_info *compptr;

  emit_marker(cinfo, M_SOS);
  if (cinfo->error_flag)
    return;

  emit_2bytes(cinfo, 2 * cinfo->comps_in_scan + 2 + 1 + 3); /* length */
  if (cinfo->error_flag)
    return;

  emit_byte(cinfo, cinfo->comps_in_scan);
  if (cinfo->error_flag)
    return;

  for (i = 0; i < cinfo->comps_in_scan; i++) {
    compptr = cinfo->cur_comp_info[i];
    emit_byte(cinfo, compptr->component_id);
    if (cinfo->error_flag)
      return;
    td = compptr->dc_tbl_no;
    ta = compptr->ac_tbl_no;
    if (cinfo->progressive_mode) {
      /* Progressive mode: only DC or only AC tables are used in one scan;
       * furthermore, Huffman coding of DC refinement uses no table at all.
       * We emit 0 for unused field(s); this is recommended by the P&M text
       * but does not seem to be specified in the standard.
       */
      if (cinfo->Ss == 0) {
	ta = 0;			/* DC scan */
	if (cinfo->Ah != 0 && !cinfo->arith_code)
	  td = 0;		/* no DC table either */
      } else {
	td = 0;			/* AC scan */
      }
    }
    emit_byte(cinfo, (td << 4) + ta);
    if (cinfo->error_flag)
      return;
  }

  emit_byte(cinfo, cinfo->Ss);
  if (cinfo->error_flag)
    return;
  emit_byte(cinfo, cinfo->Se);
  if (cinfo->error_flag)
    return;
  emit_byte(cinfo, (cinfo->Ah << 4) + cinfo->Al);
}


/* Emit a JFIF-compliant APP0 marker */
// VERIFY: 100255b9 manual
// OFFSET: 0x004AD670
static void emit_jfif_app0(j_compress_ptr cinfo) {
  /*
   * Length of APP0 block	(2 bytes)
   * Block ID			(4 bytes - ASCII "JFIF")
   * Zero byte			(1 byte to terminate the ID string)
   * Version Major, Minor	(2 bytes - major first)
   * Units			(1 byte - 0x00 = none, 0x01 = inch, 0x02 = cm)
   * Xdpu			(2 bytes - dots per unit horizontal)
   * Ydpu			(2 bytes - dots per unit vertical)
   * Thumbnail X size		(1 byte)
   * Thumbnail Y size		(1 byte)
   */

  emit_marker(cinfo, M_APP0);
  if (cinfo->error_flag)
    return;

  emit_2bytes(cinfo, 2 + 4 + 1 + 2 + 1 + 2 + 2 + 1 + 1); /* length */
  if (cinfo->error_flag)
    return;

  emit_byte(cinfo, 0x4A);	/* Identifier: ASCII "JFIF" */
  if (cinfo->error_flag)
    return;
  emit_byte(cinfo, 0x46);
  if (cinfo->error_flag)
    return;
  emit_byte(cinfo, 0x49);
  if (cinfo->error_flag)
    return;
  emit_byte(cinfo, 0x46);
  if (cinfo->error_flag)
    return;
  emit_byte(cinfo, 0);
  if (cinfo->error_flag)
    return;
  emit_byte(cinfo, cinfo->JFIF_major_version); /* Version fields */
  if (cinfo->error_flag)
    return;
  emit_byte(cinfo, cinfo->JFIF_minor_version);
  if (cinfo->error_flag)
    return;
  emit_byte(cinfo, cinfo->density_unit); /* Pixel size information */
  if (cinfo->error_flag)
    return;
  emit_2bytes(cinfo, (int) cinfo->X_density);
  if (cinfo->error_flag)
    return;
  emit_2bytes(cinfo, (int) cinfo->Y_density);
  if (cinfo->error_flag)
    return;
  emit_byte(cinfo, 0);		/* No thumbnail image */
  if (cinfo->error_flag)
    return;
  emit_byte(cinfo, 0);
}


/* Emit an Adobe APP14 marker */
// VERIFY: 47007e1b manual
// OFFSET: 0x004AD7B0
static void emit_adobe_app14(j_compress_ptr cinfo) {
  /*
   * Length of APP14 block	(2 bytes)
   * Block ID			(5 bytes - ASCII "Adobe")
   * Version Number		(2 bytes - currently 100)
   * Flags0			(2 bytes - currently 0)
   * Flags1			(2 bytes - currently 0)
   * Color transform		(1 byte)
   *
   * Although Adobe TN 5116 mentions Version = 101, all the Adobe files
   * now in circulation seem to use Version = 100, so that's what we write.
   *
   * We write the color transform byte as 1 if the JPEG color space is
   * YCbCr, 2 if it's YCCK, 0 otherwise.  Adobe's definition has to do with
   * whether the encoder performed a transformation, which is pretty useless.
   */

  emit_marker(cinfo, M_APP14);
  if (cinfo->error_flag)
    return;

  emit_2bytes(cinfo, 2 + 5 + 2 + 2 + 2 + 1); /* length */
  if (cinfo->error_flag)
    return;

  emit_byte(cinfo, 0x41);	/* Identifier: ASCII "Adobe" */
  if (cinfo->error_flag)
    return;
  emit_byte(cinfo, 0x64);
  if (cinfo->error_flag)
    return;
  emit_byte(cinfo, 0x6F);
  if (cinfo->error_flag)
    return;
  emit_byte(cinfo, 0x62);
  if (cinfo->error_flag)
    return;
  emit_byte(cinfo, 0x65);
  if (cinfo->error_flag)
    return;
  emit_2bytes(cinfo, 100);	/* Version */
  if (cinfo->error_flag)
    return;
  emit_2bytes(cinfo, 0);	/* Flags0 */
  if (cinfo->error_flag)
    return;
  emit_2bytes(cinfo, 0);	/* Flags1 */
  if (cinfo->error_flag)
    return;
  switch (cinfo->jpeg_color_space) {
  case JCS_YCbCr:
    emit_byte(cinfo, 1);	/* Color transform = 1 */
    break;
  case JCS_YCCK:
    emit_byte(cinfo, 2);	/* Color transform = 2 */
    break;
  default:
    emit_byte(cinfo, 0);	/* Color transform = 0 */
    break;
  }
}


/*
 * These routines allow writing an arbitrary marker with parameters.
 * The only intended use is to emit COM or APPn markers after calling
 * write_file_header and before calling write_frame_header.
 * Other uses are not guaranteed to produce desirable results.
 * Counting the parameter bytes properly is the caller's responsibility.
 */

/* Emit an arbitrary marker header */
// VERIFY: 0cf705fd auto
// OFFSET: 0x004AD8C0
static void write_marker_header(j_compress_ptr cinfo, int marker, unsigned int datalen) {
  if (datalen > (unsigned int) 65533)		/* safety check */ {
    ERREXIT(cinfo, JERR_BAD_LENGTH);
    return;
  }

  emit_marker(cinfo, (JPEG_MARKER) marker);
  if (cinfo->error_flag)
    return;

  emit_2bytes(cinfo, (int) (datalen + 2));	/* total length */
}

/* Emit one byte of marker parameters following write_marker_header */
// VERIFY: 8f0fe453 auto
// OFFSET: 0x004AD910
static void write_marker_byte(j_compress_ptr cinfo, int val) {
  emit_byte(cinfo, val);
}


/*
 * Write datastream header.
 * This consists of an SOI and optional APPn markers.
 * We recommend use of the JFIF marker, but not the Adobe marker,
 * when using YCbCr or grayscale data.  The JFIF marker should NOT
 * be used for any other JPEG colorspace.  The Adobe marker is helpful
 * to distinguish RGB, CMYK, and YCCK colorspaces.
 * Note that an application can write additional header markers after
 * jpeg_start_compress returns.
 */

// VERIFY: 6ff15b16 auto
// OFFSET: 0x004AD950
static void write_file_header(j_compress_ptr cinfo) {
  my_marker_ptr marker = (my_marker_ptr) cinfo->marker;

  emit_marker(cinfo, M_SOI);	/* first the SOI */
  if (cinfo->error_flag)
    return;

  /* SOI is defined to reset restart interval to 0 */
  marker->last_restart_interval = 0;

  if (cinfo->write_JFIF_header)	/* next an optional JFIF APP0 */ {
    emit_jfif_app0(cinfo);
    if (cinfo->error_flag)
      return;
  }
  if (cinfo->write_Adobe_marker) /* next an optional Adobe APP14 */
    emit_adobe_app14(cinfo);
}


/*
 * Write frame header.
 * This consists of DQT and SOFn markers.
 * Note that we do not emit the SOF until we have emitted the DQT(s).
 * This avoids compatibility problems with incorrect implementations that
 * try to error-check the quant table numbers as soon as they see the SOF.
 */

// VERIFY: 35f5d6f6 manual
// OFFSET: 0x004AD9B0
static void write_frame_header(j_compress_ptr cinfo) {
  int ci, prec;
  boolean is_baseline;
  jpeg_component_info *compptr;

  /* Emit DQT for each quantization table.
   * Note that emit_dqt() suppresses any duplicate tables.
   */
  prec = 0;
  for (ci = 0, compptr = cinfo->comp_info; ci < cinfo->num_components;
       ci++, compptr++) {
    prec += emit_dqt(cinfo, compptr->quant_tbl_no);
    if (cinfo->error_flag)
      return;
  }
  /* now prec is nonzero iff there are any 16-bit quant tables. */

  /* Check for a non-baseline specification.
   * Note we assume that Huffman table numbers won't be changed later.
   */
  if (cinfo->arith_code || cinfo->progressive_mode ||
      cinfo->data_precision != 8) {
    is_baseline = FALSE;
  } else {
    is_baseline = TRUE;
    for (ci = 0, compptr = cinfo->comp_info; ci < cinfo->num_components;
	 ci++, compptr++) {
      if (compptr->dc_tbl_no > 1 || compptr->ac_tbl_no > 1)
	is_baseline = FALSE;
    }
    if (prec && is_baseline) {
      is_baseline = FALSE;
      /* If it's baseline except for quantizer size, warn the user */
      TRACEMS(cinfo, 0, JTRC_16BIT_TABLES);
    }
  }

  /* Emit the proper SOF marker */
  if (cinfo->arith_code) {
    emit_sof(cinfo, M_SOF9);	/* SOF code for arithmetic coding */
  } else {
    if (cinfo->progressive_mode)
      emit_sof(cinfo, M_SOF2);	/* SOF code for progressive Huffman */
    else if (is_baseline)
      emit_sof(cinfo, M_SOF0);	/* SOF code for baseline implementation */
    else
      emit_sof(cinfo, M_SOF1);	/* SOF code for non-baseline Huffman file */
  }
}


/*
 * Write scan header.
 * This consists of DHT or DAC markers, optional DRI, and SOS.
 * Compressed data will be written following the SOS.
 */

// VERIFY: f459567f auto
// OFFSET: 0x004ADAB0
static void write_scan_header(j_compress_ptr cinfo) {
  my_marker_ptr marker = (my_marker_ptr) cinfo->marker;
  int i;
  jpeg_component_info *compptr;

  if (cinfo->arith_code) {
    /* Emit arith conditioning info.  We may have some duplication
     * if the file has multiple scans, but it's so small it's hardly
     * worth worrying about.
     */
    emit_dac(cinfo);
    if (cinfo->error_flag)
      return;
  } else {
    /* Emit Huffman tables.
     * Note that emit_dht() suppresses any duplicate tables.
     */
    for (i = 0; i < cinfo->comps_in_scan; i++) {
      compptr = cinfo->cur_comp_info[i];
      if (cinfo->progressive_mode) {
	/* Progressive mode: only DC or only AC tables are used in one scan */
	if (cinfo->Ss == 0) {
	  if (cinfo->Ah == 0)	/* DC needs no table for refinement scan */
	    emit_dht(cinfo, compptr->dc_tbl_no, FALSE);
	} else {
	  emit_dht(cinfo, compptr->ac_tbl_no, TRUE);
	}
      } else {
	/* Sequential mode: need both DC and AC tables */
	emit_dht(cinfo, compptr->dc_tbl_no, FALSE);
	if (cinfo->error_flag)
	  return;
	emit_dht(cinfo, compptr->ac_tbl_no, TRUE);
      }
      if (cinfo->error_flag)
	return;
    }
  }

  /* Emit DRI if required --- note that DRI value could change for each scan.
   * We avoid wasting space with unnecessary DRIs, however.
   */
  if (cinfo->restart_interval != marker->last_restart_interval) {
    emit_dri(cinfo);
    if (cinfo->error_flag)
      return;
    marker->last_restart_interval = cinfo->restart_interval;
  }

  emit_sos(cinfo);
}


/*
 * Write datastream trailer.
 */

// VERIFY: 95095f76 auto
// OFFSET: 0x004ADBC0
static void write_file_trailer(j_compress_ptr cinfo) {
  emit_marker(cinfo, M_EOI);
}


/*
 * Write an abbreviated table-specification datastream.
 * This consists of SOI, DQT and DHT tables, and EOI.
 * Any table that is defined and not marked sent_table = TRUE will be
 * emitted.  Note that all tables will be marked sent_table = TRUE at exit.
 */

// VERIFY: 04fdd1c0 manual
// OFFSET: 0x004ADBE0
static void write_tables_only(j_compress_ptr cinfo) {
  int i;

  emit_marker(cinfo, M_SOI);
  if (cinfo->error_flag)
    return;

  for (i = 0; i < NUM_QUANT_TBLS; i++) {
    if (cinfo->quant_tbl_ptrs[i] != NULL) {
      (void) emit_dqt(cinfo, i);
      if (cinfo->error_flag)
        return;
    }
  }

  if (! cinfo->arith_code) {
    for (i = 0; i < NUM_HUFF_TBLS; i++) {
      if (cinfo->dc_huff_tbl_ptrs[i] != NULL) {
	emit_dht(cinfo, i, FALSE);
	if (cinfo->error_flag)
	  return;
      }
      if (cinfo->ac_huff_tbl_ptrs[i] != NULL) {
	emit_dht(cinfo, i, TRUE);
	if (cinfo->error_flag)
	  return;
      }
    }
  }

  emit_marker(cinfo, M_EOI);
}


/*
 * Initialize the marker writer module.
 */

// VERIFY: 6123853e auto
// OFFSET: 0x004ADCF0
void jinit_marker_writer(j_compress_ptr cinfo) {
  my_marker_ptr marker;

  /* Create the subobject */
  marker = (my_marker_ptr)
    (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_IMAGE,
				SIZEOF(my_marker_writer));
  cinfo->marker = (struct jpeg_marker_writer *) marker;
  /* Initialize method pointers */
  marker->pub.write_file_header = write_file_header;
  marker->pub.write_frame_header = write_frame_header;
  marker->pub.write_scan_header = write_scan_header;
  marker->pub.write_file_trailer = write_file_trailer;
  marker->pub.write_tables_only = write_tables_only;
  marker->pub.write_marker_header = write_marker_header;
  marker->pub.write_marker_byte = write_marker_byte;
  /* Initialize private state */
  marker->last_restart_interval = 0;
}
