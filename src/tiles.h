#include "imageLoader.h"
#include <stdio.h>      // sprintf()

#ifndef SETH_B_2D_GAME
#define SETH_B_2D_GAME

#define NUM_TILE_IMAGES                     8
#define EXE_FILENAME_LENGTH                 4 + 3 // "main.exe"

#define TILE_FLAG_INITIALIZED               0x0001

#define TILE_ERR_SUCCESS                    0
#define TILE_ERR_INIT_DIRECTORY_FAILED      1
#define TILE_ERR_INIT_IMAGE_LOAD_FAILED     2
#define TILE_ERR_NOT_INITIALIZED            3
#define TILE_ERR_ALREADY_INITIALIZED        4
#define TILE_ERR_DIRECTORY_DAMAGED          5
#define TILE_ERR_NO_MEMORY                  6
#define TILE_ERR_IMAGELOADER_FAILED_INIT    7

#define TILE_IMAGE_WIDTH                    32
#define TILE_IMAGE_HEIGHT                   32

#define TILE_MAP_WIDTH                      32
#define TILE_MAP_HEIGHT                     32

#define COLOR_TRANSPARENT_VALUE             0x00FF00FF

#endif

struct tile_Image {
    //uint16_t sizeX;
    //uint16_t sizeY;
    uint32_t* pixels;
};
extern struct tile_Image tileBitmapInfos[256]; // an array of tile images corresponding to (tileId - 1) (as zero doesn't need an image)

struct tile {
    uint16_t tileId;
    uint8_t extra;
    uint8_t flags;
};
extern uint16_t        tile_mapSizeX;  // how many tiles wide the map is
extern uint16_t        tile_mapSizeY;  // how many tiles long the map is
extern struct tile*    tile_map;       // a pointer to the 2D tilemap array

uint32_t tile_init(HDC hdc, uint16_t mapSizeX, uint16_t mapSizeY);
//uint32_t tile_setMap(uint16_t mapSizeX, uint16_t mapSizeY);
//uint32_t tile_setMap(uint16_t* tileMap, uint16_t mapSizeX, uint16_t mapSizeY);
uint32_t tile_close();
uint32_t tile_getLastError();
uint32_t* tile_getBitmapFromID(uint16_t tileId);