#define JPEG_INTERNALS
#include "jinclude.h"
#include "jpeglib.h"
#include "jmemsrc.h"

// VERIFY: ab4932dd auto
// OFFSET: 0x004ABE60
static void read_pixel(mem_source_ptr sinfo) {
  int i;

  for (i = 0; i < sinfo->bytes_per_pixel; i++)
    sinfo->pixel[i] = sinfo->pixels[sinfo->position++];
}

// VERIFY: f4a5c411 manual
// OFFSET: 0x004ABE90
static JDIMENSION get_rows(j_compress_ptr cinfo, mem_source_ptr sinfo) {
  JSAMPROW outptr = sinfo->buffer[0];
  JDIMENSION col;

  for (col = cinfo->image_width; col > 0; col--) {
    (*sinfo->read_pixel) (sinfo);
    *outptr++ = (JSAMPLE) sinfo->pixel[2];
    *outptr++ = (JSAMPLE) sinfo->pixel[1];
    *outptr++ = (JSAMPLE) sinfo->pixel[0];
  }

  return 1;
}

// VERIFY: 86ebe17a auto
// OFFSET: 0x004ABEE0
static void start_input_mem(j_compress_ptr cinfo, mem_source_ptr sinfo) {
  sinfo->read_pixel = read_pixel;
  sinfo->get_rows = get_rows;
  sinfo->buffer = (*cinfo->mem->alloc_sarray) ((j_common_ptr) cinfo, JPOOL_IMAGE, (JDIMENSION) (sinfo->width * 3), (JDIMENSION) 1);
  sinfo->buffer_height = 1;
  sinfo->get_pixel_rows = sinfo->get_rows;
  cinfo->data_precision = 8;
  cinfo->image_width = sinfo->width;
  cinfo->image_height = sinfo->height;
}

// VERIFY: 5b982c5e auto
// OFFSET: 0x005EEB70
static void finish_input_mem(j_compress_ptr cinfo, mem_source_ptr sinfo) {
}

// VERIFY: bf69649c manual
// OFFSET: 0x004ABF40
mem_source_ptr jinit_read_mem(j_compress_ptr cinfo) {
  mem_source_ptr source;

  source = (mem_source_ptr) (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_IMAGE, SIZEOF(struct mem_source_struct));
  source->size = cinfo->image_width * cinfo->image_height * 4;
  source->width = cinfo->image_width;
  source->height = cinfo->image_height;
  source->position = 0;
  source->pixels = (const JOCTET *) cinfo->source_pixels;
  source->cinfo = cinfo;
  source->bytes_per_pixel = cinfo->input_components;
  source->start_input = start_input_mem;
  source->finish_input = finish_input_mem;

  return source;
}
