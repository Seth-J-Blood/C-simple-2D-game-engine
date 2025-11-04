#include "tiles.h"

struct tile_Image tileBitmapInfos[256];
uint16_t        tile_mapSizeX;  // how many tiles wide the map is
uint16_t        tile_mapSizeY;  // how many tiles long the map is
struct tile*    tile_map;       // a pointer to the 2D tilemap array

static uint16_t internal_flags = 0x0000;
static TCHAR internal_current_directory[MAX_PATH];
static TCHAR internal_directory[MAX_PATH];         // contains the directory of the .exe file and a null-terminator character
static uint32_t internal_lastError = 0;
static HDC internal_hdc;


uint32_t tile_init(HDC hdc, uint16_t mapSizeX, uint16_t mapSizeY) {
    // if tiles was already initialized, don't do anything //
    if (internal_flags & TILE_FLAG_INITIALIZED) {
        internal_lastError = TILE_ERR_ALREADY_INITIALIZED;
        return TILE_ERR_ALREADY_INITIALIZED;
    }

    uint32_t initError = imageloader_init(hdc);
    if (initError != IMGLDR_ERR_SUCCESS && initError != IMGLDR_ERR_ALREADY_INITIALIZED) {
        internal_lastError = TILE_ERR_IMAGELOADER_FAILED_INIT;
        return TILE_ERR_IMAGELOADER_FAILED_INIT;
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
        //memset(tileBitmapInfos[i].pixels, 0x00FF00FF, TILE_IMAGE_HEIGHT * TILE_IMAGE_WIDTH * sizeof(uint32_t));

        // load string "[directory]/images/tiles/floors/tile[i].bmp" into internal_current_directory. Use snprintf to prevent buffer overflow
        if (!snprintf(internal_current_directory, MAX_PATH, "%s\\images\\tiles\\tile%hu.bmp", internal_directory, i)) {
            MessageBox(NULL, "Directory failed!", "Debug box", MB_ICONEXCLAMATION);
            MessageBox(NULL, internal_current_directory, "Debug box", MB_ICONEXCLAMATION);
            continue;
        }
        
        if (imageLoader_newImage_output(internal_current_directory, TILE_IMAGE_WIDTH, TILE_IMAGE_HEIGHT, &tileBitmapInfos[i].pixels) == IMGLDR_INVALID_IMAGE) {
            MessageBox(NULL, "Directory2 failed!", "Debug box", MB_ICONEXCLAMATION);
            MessageBox(NULL, internal_current_directory, "Debug box", MB_ICONEXCLAMATION);
            TCHAR message[24];
            snprintf(message, 24, "Error: 0x%X", imageLoader_getLastError());
            MessageBox(NULL, message, "Debug box", MB_ICONEXCLAMATION);
            break;
        }
    }

    // ALLOCATE TILEMAP
    if ((tile_map = malloc(sizeof(struct tile) * mapSizeX * mapSizeY)) == NULL) {
        internal_lastError = TILE_ERR_NO_MEMORY;
        return TILE_ERR_NO_MEMORY;
    }

    // SET TILEMAP TO ALL GRASS
    for (uint32_t y = 0; y < TILE_MAP_HEIGHT; y++) {
        for (uint32_t x = 0; x < TILE_MAP_WIDTH; x++) {
            struct tile temp = {
                .tileId = 1,
                .extra = 0,
                .flags = 0
            };
            tile_map[(y * TILE_MAP_WIDTH) + x] = temp;
        }
    }
    internal_flags |= TILE_FLAG_INITIALIZED;
    internal_lastError = TILE_ERR_SUCCESS;
    return TILE_ERR_SUCCESS;
}

uint32_t tile_getLastError() {
    return internal_lastError;
}

uint32_t* tile_getBitmapFromID(uint16_t tileId) {
    if (!internal_flags & TILE_FLAG_INITIALIZED) {
        return NULL;
    }

    internal_lastError = TILE_ERR_SUCCESS;
    return tileBitmapInfos[tileId - 1].pixels;
}

uint32_t tile_close() {
    // if no initialization call has been made yet, error //
    if (!(internal_flags & TILE_FLAG_INITIALIZED)) {
        internal_lastError = TILE_ERR_NOT_INITIALIZED;
        return TILE_ERR_NOT_INITIALIZED;
    }

    // deallocate tile bitmaps
    for (int i = 0; i < 256; i++) {
        free(tileBitmapInfos[i].pixels);
    }

    // deallocate tile_map
    free(tile_map);

    internal_lastError = TILE_ERR_SUCCESS;
    return TILE_ERR_SUCCESS;
}