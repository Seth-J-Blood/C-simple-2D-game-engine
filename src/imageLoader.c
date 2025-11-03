#include "imageLoader.h"
#include <stdio.h>

#define INTERN_FLAG_INITIALIZED     0x00000001

//TCHAR       internal_active_directory[MAX_PATH];
//TCHAR       internal_current_directory[MAX_PATH];
static HDC         internal_hdc;
static uint32_t    internal_lastError = 0;
static uint32_t    internal_flags = 0x00000000;
static uint32_t*   internal_allocatedArray[IMGLDR_MAX_IMAGES_DEFINED];
static uint8_t     internal_allocatedArraySlots[IMGLDR_MAX_IMAGES_DEFINED / 8] = { 0 };
static uint32_t    internal_allocatedImages = 0;

static BITMAPINFO internal_bufferBitmapInfo = {
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

static int64_t internal_getFirstFreeSlot() {
    for (uint32_t i = 0; i < IMGLDR_MAX_IMAGES_DEFINED / 8; i++) {
        if (internal_allocatedArraySlots[i] != 0xFF) {
            // find first zero bit
            for (uint8_t mask = 7; mask > 0; mask--) {
                if (!(internal_allocatedArraySlots[i] & (1 << mask)))
                    return i * 8 + mask;
            }
        }
    }

    return -1;
}

uint32_t imageLoader_getLastError(){
    return internal_lastError;
}

uint32_t imageloader_init(HDC hdc) {
    if (internal_flags & INTERN_FLAG_INITIALIZED) {
        internal_lastError = IMGLDR_ERR_ALREADY_INITIALIZED;
        return IMGLDR_ERR_ALREADY_INITIALIZED;
    }

    //// get current directory
    //uint16_t charsWritten = GetCurrentDirectory(MAX_PATH, internal_active_directory); // this should NEVER fail for lack of memory space, but whatever
    //if (charsWritten == 0) {
    //    internal_lastError = IMGLDR_ERR_INIT_DIRECTORY_FAILED;
    //    return IMGLDR_ERR_INIT_DIRECTORY_FAILED;
    //}
    //internal_active_directory[charsWritten - 4] = 0;

    internal_flags |= INTERN_FLAG_INITIALIZED;
    internal_hdc = hdc;
    internal_allocatedImages = 0;

    internal_lastError = IMGLDR_ERR_SUCCESS;
    return IMGLDR_ERR_SUCCESS;    
}

imageHandle_t imageHandler_newImage(const PTCHAR absolute_filename, uint16_t imageWidth, uint16_t imageHeight) {
    if (!(internal_flags & INTERN_FLAG_INITIALIZED)) {
        internal_lastError = IMGLDR_ERR_NOT_INITIALIZED;
        return IMGLDR_INVALID_IMAGE;
    }

    // sanity check
    if (imageWidth == 0 || imageHeight == 0) {
        internal_lastError = IMGLDR_ERR_BAD_PARAMETERS;
        return IMGLDR_INVALID_IMAGE;
    }

    // if max images defined, error
    if (internal_allocatedImages >= IMGLDR_MAX_IMAGES_DEFINED) {
        internal_lastError = IMGLDR_ERR_NOT_INITIALIZED;
        return IMGLDR_INVALID_IMAGE;
    }

    // FIND FIRST FREE SLOT
    int64_t freeSlot = internal_getFirstFreeSlot();
    if (freeSlot == -1) {
        // uhh this should never happen, but frick it we ball
        internal_lastError = IMGLDR_ERR_NOT_INITIALIZED;
        return IMGLDR_INVALID_IMAGE;
    }

    // RESERVE RAM
    if ((internal_allocatedArray[freeSlot] = malloc(imageWidth * imageHeight * sizeof(uint32_t))) == NULL) {
        // no memory, malloc failed
        internal_lastError = IMGLDR_ERR_NO_MEMORY;
        return IMGLDR_INVALID_IMAGE;
    }
    
    // LOAD IMAGE FROM DISK
    internal_bufferBitmapInfo.bmiHeader.biWidth = imageWidth;
    internal_bufferBitmapInfo.bmiHeader.biWidth = -imageHeight;
    HBITMAP image = LoadImageA(NULL, absolute_filename, IMAGE_BITMAP, imageWidth, imageHeight, LR_LOADFROMFILE);
    if (image == NULL) {
        // whoopsie daisy load image failed
        free(internal_allocatedArray[freeSlot]);
        internal_lastError = IMGLDR_ERR_DIRECTORY_DAMAGED;
        return IMGLDR_INVALID_IMAGE;
    }

    GetDIBits(internal_hdc, image, 0, imageHeight, internal_allocatedArray[freeSlot], &internal_bufferBitmapInfo, DIB_RGB_COLORS);
    DeleteObject(image);

    internal_allocatedArraySlots[freeSlot/8] |= (0b10000000 >> (freeSlot & 0b00000111));
    internal_allocatedImages++;
    internal_lastError = IMGLDR_ERR_SUCCESS;
    return freeSlot;
}

imageHandle_t imageLoader_newImage_output(const PTCHAR absolute_filename, uint16_t imageWidth, uint16_t imageHeight, uint32_t** output) {
    if (!(internal_flags & INTERN_FLAG_INITIALIZED)) {
        internal_lastError = IMGLDR_ERR_NOT_INITIALIZED;
        return IMGLDR_INVALID_IMAGE;
    }

    // sanity check
    if (imageWidth == 0 || imageHeight == 0) {
        internal_lastError = IMGLDR_ERR_BAD_PARAMETERS;
        return IMGLDR_INVALID_IMAGE;
    }

    // if max images defined, error
    if (internal_allocatedImages >= IMGLDR_MAX_IMAGES_DEFINED) {
        internal_lastError = IMGLDR_ERR_MAX_IMAGES_DEFINED;
        return IMGLDR_INVALID_IMAGE;
    }

    // FIND FIRST FREE SLOT
    int64_t freeSlot = internal_getFirstFreeSlot();
    if (freeSlot == -1) {
        // uhh this should never happen, but frick it we ball
        internal_lastError = IMGLDR_ERR_MAX_IMAGES_DEFINED;
        return IMGLDR_INVALID_IMAGE;
    }

    // RESERVE RAM
    if ((internal_allocatedArray[freeSlot] = malloc(imageWidth * imageHeight * sizeof(uint32_t))) == NULL) {
        // no memory, malloc failed
        internal_lastError = IMGLDR_ERR_NO_MEMORY;
        return IMGLDR_INVALID_IMAGE;
    }
    
    // LOAD IMAGE FROM DISK
    internal_bufferBitmapInfo.bmiHeader.biWidth = imageWidth;
    internal_bufferBitmapInfo.bmiHeader.biHeight = -imageHeight;
    SetLastError(0);
    HBITMAP image = LoadImageA(NULL, absolute_filename, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    if (image == NULL) {
        // whoopsie daisy load image failed
        free(internal_allocatedArray[freeSlot]);
        internal_lastError = IMGLDR_ERR_DIRECTORY_DAMAGED;
        return IMGLDR_INVALID_IMAGE;
    }

    GetDIBits(internal_hdc, image, 0, imageHeight, internal_allocatedArray[freeSlot], &internal_bufferBitmapInfo, DIB_RGB_COLORS);
    *(output) = internal_allocatedArray[freeSlot];
    DeleteObject(image);

    internal_allocatedArraySlots[freeSlot/8] |= (0b10000000 >> (freeSlot & 0b00000111));
    internal_allocatedImages++;
    internal_lastError = IMGLDR_ERR_SUCCESS;
    return freeSlot;
}

uint32_t imageLoader_fetchImage(imageHandle_t image, uint32_t** output) {
    if (!(internal_flags & INTERN_FLAG_INITIALIZED)) {
        internal_lastError = IMGLDR_ERR_NOT_INITIALIZED;
        return IMGLDR_ERR_NOT_INITIALIZED;
    }

    if (!(internal_allocatedArraySlots[image / 8] & (0b10000000 >> (image & 0b00000111)))) {
        internal_lastError = IMGLDR_ERR_IMAGE_NOT_LOADED;
        return IMGLDR_ERR_IMAGE_NOT_LOADED;
    }

    // fetch image for output
    *(output) = internal_allocatedArray[image];
    internal_lastError = IMGLDR_ERR_SUCCESS;
    return IMGLDR_ERR_SUCCESS;
}

uint32_t imageLoader_deleteImage(imageHandle_t image) {
    if (!(internal_flags & INTERN_FLAG_INITIALIZED)) {
        internal_lastError = IMGLDR_ERR_NOT_INITIALIZED;
        return IMGLDR_ERR_NOT_INITIALIZED;
    }

    if (!(internal_allocatedArraySlots[image / 8] & (0b10000000 >> (image & 0b00000111)))) {
        internal_lastError = IMGLDR_ERR_IMAGE_NOT_LOADED;
        return IMGLDR_ERR_IMAGE_NOT_LOADED;
    }

    free(internal_allocatedArray[image]);
    internal_allocatedArraySlots[image / 8] &= (~(0b10000000 >> (image & 0b00000111)));

    internal_lastError = IMGLDR_ERR_SUCCESS;
    return IMGLDR_ERR_SUCCESS;
}

uint32_t imageloader_close() {
    if (!(internal_flags & INTERN_FLAG_INITIALIZED)) {
        internal_lastError = IMGLDR_ERR_NOT_INITIALIZED;
        return IMGLDR_ERR_NOT_INITIALIZED;
    }

    // DEALLOCATE ALL IMAGES FROM HEAP
    for (uint32_t i = 0; i < IMGLDR_MAX_IMAGES_DEFINED / 8; i++) {
        // could optimize this but whatever
        if (internal_allocatedArraySlots[i] != 0x00) {
            for (uint8_t mask = 7; mask >= 0; mask--) {
                if (internal_allocatedArraySlots[i] & (1 << mask)) {
                    free(internal_allocatedArray[i * 8 + (7 - mask)]);
                    internal_allocatedArraySlots[i] &= ~(1 << mask);
                }
            }
        }
    }

    internal_flags = 0;
    internal_lastError = IMGLDR_ERR_SUCCESS;
    return IMGLDR_ERR_SUCCESS;
}