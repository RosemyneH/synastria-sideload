#define JPEG_INTERNALS
#include "jinclude.h"
#include "jpeglib.h"
#include "jmemsys.h"
#include "jmemnobs.h"

// STATIC: 0x00B49C6C
static void* (*s_jpegAlloc)(size_t bytes);

// STATIC: 0x00B49C70
static void (*s_jpegFree)(void* ptr);

// VERIFY: 19731a34 auto
// OFFSET: 0x004AAF60
void jpeg_set_malloc(void* (*allocFunc)(size_t bytes)) {
    s_jpegAlloc = allocFunc;
}

// VERIFY: 3e6a670f auto
// OFFSET: 0x004AAF70
void jpeg_set_free(void (*freeFunc)(void* ptr)) {
    s_jpegFree = freeFunc;
}

// VERIFY: 2b11fb05 auto
// OFFSET: 0x004AAFA0
void* jpeg_get_small(j_common_ptr cinfo, size_t sizeofobject) {
    if (s_jpegAlloc) {
        return s_jpegAlloc(sizeofobject);
    }

    return NULL;
}

// VERIFY: adc71443 auto
// OFFSET: 0x004AAF80
void jpeg_free_small(j_common_ptr cinfo, void* object, size_t sizeofobject) {
    if (s_jpegFree) {
        s_jpegFree(object);
    }
}

// VERIFY: d396333e auto
// OFFSET: 0x004AAFA0
void* jpeg_get_large(j_common_ptr cinfo, size_t sizeofobject) {
    if (s_jpegAlloc) {
        return s_jpegAlloc(sizeofobject);
    }

    return NULL;
}

// VERIFY: b8e41fb8 auto
// OFFSET: 0x004AAF80
void jpeg_free_large(j_common_ptr cinfo, void* object, size_t sizeofobject) {
    if (s_jpegFree) {
        s_jpegFree(object);
    }
}

// VERIFY: 4d0a9f70 auto
// OFFSET: 0x004AAFC0
long jpeg_mem_available(j_common_ptr cinfo, long min_bytes_needed, long max_bytes_needed, long already_allocated) {
    return max_bytes_needed;
}

// VERIFY: a43cad58 auto
// OFFSET: 0x004AAFD0
void jpeg_open_backing_store(j_common_ptr cinfo, backing_store_ptr info, long total_bytes_needed) {
    ERREXIT(cinfo, JERR_NO_BACKING_STORE);
}

// VERIFY: 4516d084 auto
// OFFSET: 0x008E5250
long jpeg_mem_init(j_common_ptr cinfo) {
    return 0;
}

// VERIFY: 61e9e9d9 auto
// OFFSET: 0x005EEB70
void jpeg_mem_term(j_common_ptr cinfo) {
}
