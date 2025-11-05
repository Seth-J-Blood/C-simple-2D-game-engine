#include "imageLoader.h"

#ifndef __ENTITIES_INCLUDE_GUARD__
#define __ENTITIES_INCLUDE_GUARD__

    #ifndef ENTITY_MAX_ENTITIES_ALIVE
        #define ENTITY_MAX_ENTITIES_ALIVE               256     // MUST BE A MULTIPLE OF 8
    #endif  

#define ENTITY_ERR_SUCCESS                      0
#define ENTITY_ERR_INIT_DIRECTORY_FAILED        1
#define ENTITY_ERR_INIT_IMAGE_LOAD_FAILED       2
#define ENTITY_ERR_NOT_INITIALIZED              3
#define ENTITY_ERR_ALREADY_INITIALIZED          4
#define ENTITY_ERR_DIRECTORY_DAMAGED            5
#define ENTITY_ERR_NO_MEMORY                    6
#define ENTITY_ERR_IMAGELOADER_FAILED_INIT      7
#define ENTITY_ERR_INVALID_ENTITY               8
#define ENTITY_ERER_TOO_MANY_ENTITIES           9

#define ENTITY_INVALID_ENTITY                   -1

#define ENTITY_FLAG_IS_VALID                    1
#define ENTITY_FLAG_IS_MOVING                   2
#define ENTITY_FLAG_IS_FACING_LEFT              4

typedef uint32_t entity_t;

struct entity {
    uint32_t x;
    uint32_t y;
    uint32_t imgX;
    uint32_t imgY;
    imageHandle_t image;
    uint8_t flags;
    uint8_t id;
    uint8_t rotation;
    uint8_t ticksScared;
    uint8_t ticksSinceFrameChange;
    uint8_t frameNum;
};

uint32_t entity_init();
uint32_t entity_close();
entity_t entity_spawn(uint32_t x, uint32_t y, struct entity newEntity);
uint32_t entity_delete(entity_t entity);
extern struct entity entityList[ENTITY_MAX_ENTITIES_ALIVE];

#endif