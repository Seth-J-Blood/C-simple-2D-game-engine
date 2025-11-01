#include "imageLoader.h"

#define INTERN_FLAG_INITIALIZED     0x00000001

TCHAR       internal_current_directory[MAX_PATH];
HDC         internal_hdc;
uint32_t    internal_lastError = 0;
uint32_t    internal_flags = 0x00000000;
uint32_t    intern_allocatedArray[IMGLDR_MAX_IMAGES_DEFINED];
uint8_t     intern_filledAllocatedArraySlots[IMGLDR_MAX_IMAGES_DEFINED / 8];

BITMAPINFO internal_bufferBitmapInfo = {
    .bmiHeader = {
        .biSize = sizeof(BITMAPINFOHEADER),
        .biWidth = 0,
        .biHeight = 0,      // MAKE NEGATIVE
        .biPlanes = 1,
        .biBitCount = 32,
        .biCompression = BI_RGB,
        .biSizeImage = 0,   // DON'T NEED TO FILL OUT - UNCOMPRESSED IMAGE
        .biXPelsPerMeter = 0,
        .biYPelsPerMeter = 0,
        .biClrUsed = 0,
        .biClrImportant = 0
    },
    .bmiColors = 0
};

uint32_t imageloader_init(HDC hdc) {
    if (internal_flags & INTERN_FLAG_INITIALIZED) {
        internal_lastError = IMGLDR_ERR_ALREADY_INITIALIZED;
        return IMGLDR_ERR_ALREADY_INITIALIZED;
    }

    internal_flags |= INTERN_FLAG_INITIALIZED;
    internal_hdc = hdc;

    return IMGLDR_ERR_SUCCESS;    
}

uint32_t imageloader_