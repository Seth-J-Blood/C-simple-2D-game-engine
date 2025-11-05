#include "entities.h"

#define INTERNAL_FLAG_INITIALIZED   0x00000001

struct entity entityList[ENTITY_MAX_ENTITIES_ALIVE];

static uint8_t internal_slotEntityList[ENTITY_MAX_ENTITIES_ALIVE / 8] = { 0 };
static uint32_t internal_numAliveEntities = 0;
static uint32_t internal_lastError = 0;
static uint32_t internal_flags = 0;

static int64_t internal_getFirstFreeSlot() {
    for (uint32_t i = 0; i < IMGLDR_MAX_IMAGES_DEFINED / 8; i++) {
        if (internal_slotEntityList[i] != 0xFF) {
            // find first zero bit
            for (uint8_t mask = 0; mask < 8; mask++) {
                if (!((internal_slotEntityList[i]) & (1 << mask)))
                    return i * 8 + mask;
            }
        }
    }

    return -1;
}

uint32_t entity_init() {
    if (internal_flags & INTERNAL_FLAG_INITIALIZED) {
        internal_lastError = ENTITY_ERR_ALREADY_INITIALIZED;
        return ENTITY_ERR_ALREADY_INITIALIZED;
    }

    // clear entity list
    struct entity e = {
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0
    };
    for (uint32_t i = 0; i < ENTITY_MAX_ENTITIES_ALIVE; i++) {
        entityList[i] = e;
    }

    internal_numAliveEntities = 0;
    internal_lastError = ENTITY_ERR_SUCCESS;
    internal_flags |= INTERNAL_FLAG_INITIALIZED;
    return ENTITY_ERR_SUCCESS;
}

uint32_t entity_close() {
    if (!(internal_flags & INTERNAL_FLAG_INITIALIZED)) {
        internal_lastError = ENTITY_ERR_NOT_INITIALIZED;
        return ENTITY_ERR_NOT_INITIALIZED;
    }

    // clear out entity allocator bitfield
    for (uint32_t i = 0; i < ENTITY_MAX_ENTITIES_ALIVE / 8; i++) {
        internal_slotEntityList[i] = 0;
    }

    internal_numAliveEntities = 0;
    internal_lastError = ENTITY_ERR_SUCCESS;
    internal_flags &= ~(INTERNAL_FLAG_INITIALIZED);
    return ENTITY_ERR_SUCCESS;
}

entity_t entity_spawn(uint32_t x, uint32_t y, struct entity newEntity) {
    if (!(internal_flags & INTERNAL_FLAG_INITIALIZED)) {
        internal_lastError = ENTITY_ERR_NOT_INITIALIZED;
        return ENTITY_INVALID_ENTITY;
    }

    if (internal_numAliveEntities >= ENTITY_MAX_ENTITIES_ALIVE) {
        internal_lastError = ENTITY_ERER_TOO_MANY_ENTITIES;
        return ENTITY_INVALID_ENTITY;
    }

    // find first free slot for entity
    int64_t freeSlot = internal_getFirstFreeSlot();
    if (freeSlot == -1) {
        internal_lastError = ENTITY_ERER_TOO_MANY_ENTITIES;
        return ENTITY_INVALID_ENTITY;
    }

    // copy entity :D
    newEntity.x = x;
    newEntity.y = y;
    internal_numAliveEntities++;
    internal_slotEntityList[freeSlot / 8] |= ((1 << (freeSlot & 0b00000111)));
    entityList[freeSlot] = newEntity;
    entityList[freeSlot].flags |= ENTITY_FLAG_IS_VALID;
    
    internal_lastError = ENTITY_ERR_SUCCESS;
    return freeSlot;
}