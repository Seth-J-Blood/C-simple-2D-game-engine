#include "tiles.h"
#include <string.h>     // string copy, string concatenate
#include <stdio.h>      // sprintf()

uint16_t internal_flags = 0x0000;
TCHAR internal_directory[MAX_PATH];         // contains the directory of the .exe file and a null-terminator character
uint32_t internal_lastError = 0;
TCHAR internal_current_directory[MAX_PATH];
HDC internal_hdc;

BITMAPINFO internal_bufferBitmapInfo = {
    .bmiHeader = {
        .biSize = sizeof(BITMAPINFOHEADER),
        .biWidth = TILE_IMAGE_WIDTH,
        .biHeight = -TILE_IMAGE_WIDTH,
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

uint32_t tile_init(HDC hdc, uint16_t mapSizeX, uint16_t mapSizeY) {
    // if tiles was already initialized, don't do anything //
    if (internal_flags & TILE_FLAG_INITIALIZED) {
        internal_lastError = TILE_ERR_ALREADY_INITIALIZED;
        return TILE_ERR_ALREADY_INITIALIZED;
    }

    internal_hdc = hdc;

    // INITIALIZE TILES FOLDER //
    // LOAD DIRECTORY //
    uint16_t charsWritten = GetCurrentDirectory(MAX_PATH, internal_directory); // this should NEVER fail for lack of memory space, but whatever
    if (charsWritten == 0) {
        internal_lastError = TILE_ERR_INIT_DIRECTORY_FAILED;
        return TILE_ERR_INIT_DIRECTORY_FAILED;
    }
    
    // chop off EXE_FILENAME_LENGTH + 1 characters from the end of the directory to get the root folder //
    internal_directory[charsWritten - 1 - (3)] = 0;     // -1 for changing number to index

    // LOAD IMAGES INTO MEMORY //   
    for (uint16_t i = 0; i < NUM_TILE_IMAGES; i++) {
        // reserve space for tileBitmapInfos.pixels in memory //
        tileBitmapInfos[i].pixels = malloc(TILE_IMAGE_HEIGHT * TILE_IMAGE_WIDTH * sizeof(uint32_t));
        if (tileBitmapInfos[i].pixels == NULL) {
            internal_lastError = TILE_ERR_NO_MEMORY;
            return TILE_ERR_NO_MEMORY;
        }

        //memset(tileBitmapInfos[i].pixels, 0x00FF00FF, TILE_IMAGE_HEIGHT * TILE_IMAGE_WIDTH * sizeof(uint32_t));

        // load string "[directory]/images/tiles/floors/tile[i].bmp" into internal_current_directory. Use snprintf to prevent buffer overflow
        if (!snprintf(internal_current_directory, MAX_PATH, "%s\\images\\tiles\\tile%hu.bmp", internal_directory, i)) {
            MessageBox(NULL, "Directory failed!", "Debug box", MB_ICONEXCLAMATION);
            MessageBox(NULL, internal_current_directory, "Debug box", MB_ICONEXCLAMATION);
            continue;
        }
        HBITMAP image = LoadImageA(NULL, internal_current_directory, IMAGE_BITMAP, TILE_IMAGE_WIDTH, TILE_IMAGE_HEIGHT, LR_LOADFROMFILE);
        if (image == NULL) {
            break;
        }

        //tileBitmapInfos[i].sizeX = TILE_IMAGE_WIDTH;
        //tileBitmapInfos[i].sizeY = TILE_IMAGE_HEIGHT;
        GetDIBits(hdc, image, 0, TILE_IMAGE_HEIGHT, tileBitmapInfos[i].pixels, &internal_bufferBitmapInfo, DIB_RGB_COLORS);

        DeleteObject(image);
    }

    // ALLOCATE TILEMAP
    if ((tile_map = malloc(sizeof(struct tile) * mapSizeX * mapSizeY)) == NULL) {
        internal_lastError = TILE_ERR_NO_MEMORY;
        return TILE_ERR_NO_MEMORY;
    }

    // SET TILEMAP TO ALL GRASS
    memset(tile_map, 0x00010000, sizeof(struct tile) * mapSizeX * mapSizeY);

    internal_flags |= TILE_FLAG_INITIALIZED;
    return TILE_ERR_SUCCESS;
}

uint32_t tile_getLastError() {
    return internal_lastError;
}

uint32_t* tile_getBitmapFromID(uint16_t tileId) {
    if (!internal_flags & TILE_FLAG_INITIALIZED) {
        return NULL;
    }

    return tileBitmapInfos[tileId - 1].pixels;
}

uint32_t tile_close() {
    // if no initialization call has been made yet, error //
    if (!(internal_flags & TILE_FLAG_INITIALIZED)) {
        internal_lastError = TILE_ERR_NOT_INITIALIZED;
        return TILE_ERR_NOT_INITIALIZED;
    }

    // deallocate tile bitmaps
    for (int i = 0; i < 255; i++) {
        free(tileBitmapInfos[i].pixels);
    }

    // deallocate tile_map
    free(tile_map);

    return TILE_ERR_SUCCESS;
}