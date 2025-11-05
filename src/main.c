#include "tiles.h"
#include "entities.h"
#include "imageLoader.h"

#define WANTED_FPS                  30

#define VIEWPORT_WIDTH              8
#define VIEWPORT_HEIGHT             8

#define GFLAG_MOVE_UP               0x00000001
#define GFLAG_MOVE_LEFT             0x00000002
#define GFLAG_MOVE_DOWN             0x00000004
#define GFLAG_MOVE_RIGHT            0x00000008
#define GFLAG_KILL_ANIM_ACTIVE      0x00000010
#define ANY_MOVE_FLAG               (GFLAG_MOVE_UP | GFLAG_MOVE_LEFT | GFLAG_MOVE_DOWN | GFLAG_MOVE_RIGHT)

#define FRANK_MOVE_SPEED            4
#define WALK_ANIM_TICKS_PER_SECOND  WANTED_FPS / 15


// GLOBAL VARIABLES //
HWND winHandle;             // a handle to the window associated with this program
HDC bufferHdc;              // a device context used to buffer the bitmap updates
HBITMAP bufferHandle;       // a handle to the bitmap used to buffer screen updates
const RECT dispDimensions = {
    .left = 0,
    .top = 0,
    .right = TILE_IMAGE_WIDTH * VIEWPORT_WIDTH,
    .bottom = TILE_IMAGE_HEIGHT * VIEWPORT_HEIGHT
};
BITMAPINFO bufferBitmapInfo = {
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
const CHAR* wndClassName = "2DSethGame2025";
const CHAR* wndTitle = "Seth Blood's 2D Game Engine";
TCHAR working_directory[MAX_PATH];         // contains the working directory to the 2D_GAME folder
TCHAR current_directory[MAX_PATH];         // used to navigate directories
UINT_PTR FPSTimerId = 1;

uint16_t dispSizeX = 0;         // how wide the window is, in pixels
uint16_t dispSizeY = 0;         // how tall the window is, in pixels
uint16_t screenSizeX = 0;       // how big the maximum width is, in pixels
uint16_t screenSizeY = 0;       // how big the maximum height is, in pixels
uint32_t* bufferBitmap = NULL;  // an array that contains the values of the buffer bitmap
uint32_t isStarted = 0;
uint32_t camX = 0;
uint32_t camY = 0;
entity_t frank;
entity_t william;
uint32_t gFlags = 0;

imageHandle_t frankWalkAnim[3] = {IMGLDR_INVALID_IMAGE, IMGLDR_INVALID_IMAGE, IMGLDR_INVALID_IMAGE};
imageHandle_t williamDeathAnim[2] = {IMGLDR_INVALID_IMAGE, IMGLDR_INVALID_IMAGE};

// FUNCTION PROTOTYPES //
void cleanup();
void refreshScreen();
uint32_t coordToOffset(uint16_t x, uint16_t y, uint16_t xMax);
void showLastError(const char*);

uint8_t helper_canMoveHere(int64_t newX, int64_t newY, struct entity* e) {
    int32_t tileLeft   = (newX + 8) / TILE_IMAGE_WIDTH;
    int32_t tileRight  = (newX + e->imgX - 9) / TILE_IMAGE_WIDTH;
    int32_t tileTop    = (newY + 8) / TILE_IMAGE_HEIGHT;
    int32_t tileBottom = (newY + e->imgY - 9) / TILE_IMAGE_HEIGHT;

    // Clamp to map bounds (avoid OOB reads)
    if (tileLeft < 0 || tileTop < 0 ||
        tileRight >= TILE_MAP_WIDTH || tileBottom >= TILE_MAP_HEIGHT)
        return FALSE;

    // Check all intersecting tiles
    for (int ty = tileTop; ty <= tileBottom; ty++) {
        for (int tx = tileLeft; tx <= tileRight; tx++) {
            struct tile t = tile_map[coordToOffset(tx, ty, TILE_MAP_WIDTH)];
            if (t.tileId >= 9) return FALSE;  // blocked tile
        }
    }
    return TRUE;
}

VOID CALLBACK TimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime) {
    if (!isStarted) return;

    struct entity* franky = &entityList[frank];

    if (gFlags & GFLAG_KILL_ANIM_ACTIVE) {
        if (franky->ticksSinceFrameChange < 40) {
            franky->image = frankWalkAnim[2];
        } else if (franky->ticksSinceFrameChange < 80) {
            franky->image = frankWalkAnim[0];
            entityList[william].image = williamDeathAnim[1];
        } else if (franky->ticksSinceFrameChange < 82) {
            franky->flags &= ~ENTITY_FLAG_IS_FACING_LEFT;
            franky->image = frankWalkAnim[1];
            franky->x += FRANK_MOVE_SPEED;
        } else if (franky->ticksSinceFrameChange < 84) {
            franky->image = frankWalkAnim[0];
            franky->x += FRANK_MOVE_SPEED;
        } else if (franky->ticksSinceFrameChange < 86) {
            franky->image = frankWalkAnim[1];
            franky->x += FRANK_MOVE_SPEED;
        } else if (franky->ticksSinceFrameChange < 88) {
            franky->image = frankWalkAnim[0];
            franky->x += FRANK_MOVE_SPEED;
        } else if (franky->ticksSinceFrameChange < 90) {
            franky->image = frankWalkAnim[1];
            franky->x += FRANK_MOVE_SPEED;
        } else if (franky->ticksSinceFrameChange < 92) {
            franky->image = frankWalkAnim[0];
            franky->x += FRANK_MOVE_SPEED;
        } else if (franky->ticksSinceFrameChange < 94) {
            franky->image = frankWalkAnim[1];
            franky->x += FRANK_MOVE_SPEED;
        } else if (franky->ticksSinceFrameChange < 96) {
            franky->image = frankWalkAnim[0];
            franky->x += FRANK_MOVE_SPEED;
        }

        franky->ticksSinceFrameChange++;
        refreshScreen();
        InvalidateRect(winHandle, &dispDimensions, FALSE);
        return;
    }


    // CHECK MOVEMENT FLAGS, IF ANY ARE TRUE MOVE FRANK
    if ((gFlags & GFLAG_MOVE_UP)) {
        int64_t newY = franky->y - FRANK_MOVE_SPEED;
        if (newY < 0) newY = 0;
        if (helper_canMoveHere(franky->x, newY, franky))
            franky->y = newY;
    }

    if ((gFlags & GFLAG_MOVE_DOWN)) {
        int64_t newY = franky->y + FRANK_MOVE_SPEED;
        if (newY > TILE_MAP_HEIGHT * TILE_IMAGE_HEIGHT - (franky->imgY - 8))
            newY = TILE_MAP_HEIGHT * TILE_IMAGE_HEIGHT - (franky->imgY - 8);
        if (helper_canMoveHere(franky->x, newY, franky))
            franky->y = newY;
    }

    if ((gFlags & GFLAG_MOVE_LEFT)) {
        int64_t newX = franky->x - FRANK_MOVE_SPEED;
        if (newX < 0) newX = 0;
        if (helper_canMoveHere(newX, franky->y, franky))
            franky->x = newX;
        franky->flags |= ENTITY_FLAG_IS_FACING_LEFT;
    }

    if ((gFlags & GFLAG_MOVE_RIGHT)) {
        int64_t newX = franky->x + FRANK_MOVE_SPEED;
        if (newX > TILE_MAP_WIDTH * TILE_IMAGE_WIDTH - (franky->imgX - 8))
            newX = TILE_MAP_WIDTH * TILE_IMAGE_WIDTH - (franky->imgX - 8);
        if (helper_canMoveHere(newX, franky->y, franky))
            franky->x = newX;
        franky->flags &= ~ENTITY_FLAG_IS_FACING_LEFT;
    }

    // if frank isn't moving, don't bother with animation stuff
    if (!(gFlags & ANY_MOVE_FLAG)) {
        franky->ticksSinceFrameChange = 0;
        franky->frameNum = 1;
        franky->image = frankWalkAnim[0];
    } else {

        // UPDATE FRAMES
        if (franky->ticksSinceFrameChange >= WALK_ANIM_TICKS_PER_SECOND) {
            franky->ticksSinceFrameChange = 0;
            if (franky->frameNum == 0) {
                franky->frameNum = 1;
            } else {
                franky->frameNum = 0;
            }
        }
        franky->ticksSinceFrameChange++;
        franky->image = frankWalkAnim[franky->frameNum];
    }

    // HANDLE CAMERA MOVEMENT
    int64_t relativeCamX = ((int64_t) (franky->x)) - (VIEWPORT_WIDTH * TILE_IMAGE_WIDTH)/2 + (franky->imgX / 2);
    if (relativeCamX < 0) 
        relativeCamX = 0;
    else if (relativeCamX >= (TILE_MAP_WIDTH * TILE_IMAGE_WIDTH) - (VIEWPORT_WIDTH * TILE_IMAGE_WIDTH))
        relativeCamX = ((TILE_MAP_WIDTH * TILE_IMAGE_WIDTH) - (VIEWPORT_WIDTH * TILE_IMAGE_WIDTH));
    camX = relativeCamX;


    int64_t relativeCamY = ((int64_t) (franky->y)) - (VIEWPORT_HEIGHT * TILE_IMAGE_HEIGHT)/2 + (franky->imgY / 2);
    if (relativeCamY < 0) 
        relativeCamY = 0;
    else if (relativeCamY >= (TILE_MAP_HEIGHT * TILE_IMAGE_HEIGHT) - (VIEWPORT_HEIGHT * TILE_IMAGE_HEIGHT))
        relativeCamY = ((TILE_MAP_HEIGHT * TILE_IMAGE_HEIGHT) - (VIEWPORT_HEIGHT * TILE_IMAGE_HEIGHT));
    camY = relativeCamY;

    refreshScreen();
    InvalidateRect(winHandle, &dispDimensions, FALSE);
} 

LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_KEYDOWN: {
            switch (wParam) {
                case VK_UP:
                case 'W':
                    gFlags |= GFLAG_MOVE_UP;
                break;

                case VK_LEFT:
                case 'A':
                    gFlags |= GFLAG_MOVE_LEFT;
                break;

                case VK_DOWN:
                case 'S':
                    gFlags |= GFLAG_MOVE_DOWN;
                break;

                case VK_RIGHT:
                case 'D':
                    gFlags |= GFLAG_MOVE_RIGHT;
                break;

                case 'E':
                    gFlags |= GFLAG_KILL_ANIM_ACTIVE;
                    entityList[frank].ticksSinceFrameChange = 0;
                    entityList[frank].image = frankWalkAnim[2];
                break;

                case 'R':
                    {
                        TCHAR string[60];
                        snprintf(string, 60, "CharX: %u, CharY: %u, CamX: %u, CamY: %u", entityList[frank].x, entityList[frank].y, camX, camY);
                        MessageBox(winHandle, string, "Debug Box", MB_ICONWARNING);

                        for (uint32_t i = 0; i < ENTITY_MAX_ENTITIES_ALIVE; i++) {
                            if (entityList[i].flags & ENTITY_FLAG_IS_VALID)
                                MessageBox(winHandle, "Found an entity", "Debug Box", MB_ICONWARNING);
                        }
                    }
                break;
            }
        }
        return 0;
        case WM_KEYUP: {
            switch (wParam) {
                case VK_UP:
                case 'W':
                    gFlags &= ~GFLAG_MOVE_UP;
                break;

                case VK_LEFT:
                case 'A':
                    gFlags &= ~GFLAG_MOVE_LEFT;
                break;

                case VK_DOWN:
                case 'S':
                    gFlags &= ~GFLAG_MOVE_DOWN;
                break;

                case VK_RIGHT:
                case 'D':
                    gFlags &= ~GFLAG_MOVE_RIGHT;
                break;
            }
        }
        return 0;
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            // PAINT WINDOW, DO NOT PAINT AFTER ENDPAINT //

            BitBlt(hdc, 0, 0, dispSizeX, dispSizeY, bufferHdc, 0, 0, SRCCOPY);


            // STOP PAINTING //
            EndPaint(hwnd, &ps);
        }
        return 0;
        case WM_CLOSE: {
            DestroyWindow(hwnd);
        }
        return 0;
        case WM_DESTROY: {
            // CLEANUP //
            cleanup();
            PostQuitMessage(0);
        }
        return 0;
        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdline, int cmdshow) {
    /*
        SETUP CODE
        Use this area (down to the REGISTER WINDOW CLASS comment) to set up any code
    */

    // GET SCREEN SIZE //
    screenSizeX = GetSystemMetrics(SM_CXFULLSCREEN);
    dispSizeX = screenSizeX;
    bufferBitmapInfo.bmiHeader.biWidth = screenSizeX;

    screenSizeY = GetSystemMetrics(SM_CYFULLSCREEN);
    dispSizeY = screenSizeY;
    bufferBitmapInfo.bmiHeader.biHeight = -screenSizeY;     // negative height for top-down bitmap

    // REGISTER WINDOW CLASS //
    WNDCLASSEX windowDescription = {
        .cbSize = sizeof(WNDCLASSEX),
        .style = CS_HREDRAW | CS_VREDRAW,       // redraws screen if a vertical/horizontal size change occurs
        .lpfnWndProc = WndProc,                 // what code runs on a message to the window
        .cbClsExtra = 0,
        .cbWndExtra = 0,
        .hInstance = hInst,                     // current running instance
        .hIcon = NULL,                          // icon for the window icon (top-left of window)
        .hCursor = LoadCursor(NULL, IDC_ARROW), // basic cursor
        .hbrBackground = NULL,                  // no predefined background
        .lpszMenuName = NULL,
        .lpszClassName = wndClassName,          // no L macro because UNICODE is not defined, meaning this string is 1-byte chars
        .hIcon = NULL
    };

    if (!RegisterClassEx(&windowDescription)) {         // register class so we can make the window actually exist :O
        showLastError("Window class could not be registered!");
        cleanup();
        return -1;
    }

    // CREATE WINDOW //
    winHandle = CreateWindowEx(
        0,
        windowDescription.lpszClassName,
        wndTitle,
        WS_TILED | WS_MINIMIZEBOX | WS_SYSMENU | WS_VISIBLE,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        VIEWPORT_WIDTH * TILE_IMAGE_WIDTH + 6,
        VIEWPORT_HEIGHT * TILE_IMAGE_HEIGHT + GetSystemMetrics(SM_CYCAPTION) + 6,
        NULL,
        NULL,
        hInst,
        0
    );
    if (winHandle == NULL) {
        showLastError("Window could not be created!");
        cleanup();
        return -1;
    }

    ShowWindow(winHandle, SW_SHOW);

    // SET UP BITMAP BUFFER //
    if ((bufferBitmap = (uint32_t*) malloc(screenSizeX * screenSizeY * 4)) == NULL) {
        MessageBox(NULL, "Error: could not reserve memory for DIB bitmap!", "Debug box", MB_ICONWARNING);
        cleanup();
        return -1;
    }
    bufferHdc = CreateCompatibleDC(NULL);
    bufferHandle = CreateDIBSection(bufferHdc, &bufferBitmapInfo, DIB_RGB_COLORS, (void*) &bufferBitmap, NULL, 0);
    if (bufferBitmap == NULL) {
        MessageBox(NULL, "Error: could not create a compatible bitmap handle for buffer HDC!", "Debug box", MB_ICONWARNING);
        cleanup();
        return -1;
    }
    SelectObject(bufferHdc, bufferHandle);

    UpdateWindow(winHandle);

    // INITIALIZE VARIABLES //
    // LOAD DIRECTORY //
    uint16_t charsWritten = GetCurrentDirectory(MAX_PATH, working_directory); // this should NEVER fail for lack of memory space, but whatever
    if (charsWritten == 0) {
        MessageBox(NULL, "Error: failed to get working directory!", "Debug box", MB_ICONWARNING);
        cleanup();
        return -1;
    }
    
    // chop off the number of character in '/bin' end of the directory to get the root folder //
    working_directory[charsWritten - 4] = 0;     // -1 for changing number to index
    
    // INITIALIZE TILES
    uint32_t errorCode = 0;
    if ((errorCode = tile_init(bufferHdc, TILE_MAP_WIDTH, TILE_MAP_HEIGHT)) != TILE_ERR_SUCCESS) {
        MessageBox(NULL, "Error: tile_init failed!", "Debug box", MB_ICONWARNING);
        cleanup();
        return -1;
    }
    if (tileBitmapInfos[0].pixels == NULL) {
        MessageBox(NULL, "Error: tileBitMapInfo didn't initialize!", "Debug box", MB_ICONWARNING);
        cleanup();
        return -1;
    }

    // INITIALIZE ENTITIES
    errorCode = 0;
    if ((errorCode = entity_init()) != ENTITY_ERR_SUCCESS) {
        MessageBox(NULL, "Error: entity_init failed!", "Debug box", MB_ICONWARNING);
        cleanup();
        return -1;
    }

    struct entity newEntity = {
        .x = 0,
        .y = 0,
        .imgX = 32,
        .imgY = 32,
        .image = IMGLDR_INVALID_IMAGE,
        .flags = 0,
        .id = 0,
        .rotation = 0,
        .ticksScared = 0,
        .ticksSinceFrameChange = 0,
        .frameNum = 0
    };

    // CREATE WILLIAM ENTITY //
    snprintf(current_directory, MAX_PATH, "%s%s", working_directory, "/images/entities/william-alive.bmp");
    williamDeathAnim[0] = imageLoader_newImage(
        current_directory,
        32,
        32
    );
    if ( williamDeathAnim[0] == IMGLDR_INVALID_IMAGE) {
        MessageBox(NULL, "Error: could not load william alive image!", "Debug box", MB_ICONWARNING);
        cleanup();
        return -1;
    }
    newEntity.image = williamDeathAnim[0];

    snprintf(current_directory, MAX_PATH, "%s%s", working_directory, "/images/entities/william-deaad.bmp");
    williamDeathAnim[1] = imageLoader_newImage(
        current_directory,
        32,
        32
    );
    if (williamDeathAnim[1] == IMGLDR_INVALID_IMAGE) {
        MessageBox(NULL, "Error: could not load william alive image!", "Debug box", MB_ICONWARNING);
        cleanup();
        return -1;
    }
    william = entity_spawn(992, 608, newEntity);
    if (william == ENTITY_INVALID_ENTITY) {
        MessageBox(NULL, "Error: could not spawn william!", "Debug box", MB_ICONWARNING);
        cleanup();
        return -1;
    }

    // CREATE FRANKENSTEIN ENTITY //
    snprintf(current_directory, MAX_PATH, "%s%s", working_directory, "/images/entities/freakystein_idle.bmp");
    imageHandle_t frankyImage = imageLoader_newImage(
        current_directory,
        32,
        32
    );
    if (frankyImage == IMGLDR_INVALID_IMAGE) {
        MessageBox(NULL, "Error: could not load franky image!", "Debug box", MB_ICONWARNING);
        cleanup();
        return -1;
    }
    frankWalkAnim[0] = frankyImage;
    newEntity.image = frankyImage;
    frank = entity_spawn(992, 608, newEntity);

    snprintf(current_directory, MAX_PATH, "%s%s", working_directory, "/images/entities/freakystein_move.bmp");
    frankWalkAnim[1] = imageLoader_newImage(
        current_directory,
        32,
        32
    );
    if (frankWalkAnim[1] == IMGLDR_INVALID_IMAGE) {
        MessageBox(NULL, "Error: could not load franky move image!", "Debug box", MB_ICONWARNING);
        cleanup();
        return -1;
    }

    snprintf(current_directory, MAX_PATH, "%s%s", working_directory, "/images/entities/freakystein_kill.bmp");
    frankWalkAnim[2] = imageLoader_newImage(
        current_directory,
        32,
        32
    );
    if (frankWalkAnim[2] == IMGLDR_INVALID_IMAGE) {
        MessageBox(NULL, "Error: could not load franky kill image!", "Debug box", MB_ICONWARNING);
        cleanup();
        return -1;
    }
    
    // DONE CREATING ENTITIES //

    isStarted = 1;

    // SET UP FPS TIMER
    SetTimer(winHandle, FPSTimerId, 1000 / WANTED_FPS, &TimerProc);

    // SET UP MAIN MESSAGE LOOP //
    MSG winMsg;
    InvalidateRect(winHandle, &dispDimensions, FALSE);
    while (GetMessage(&winMsg, winHandle, 0, 0)) 
        DispatchMessage(&winMsg);

    // CLEANUP //
    cleanup();
    return 0;
}

void cleanup() {
    DeleteDC(bufferHdc);
    DeleteObject(bufferBitmap);
    free(bufferBitmap);
}

void refreshScreen() {
    if (!isStarted)
        return;

    if ((gFlags & GFLAG_KILL_ANIM_ACTIVE) && entityList[frank].ticksSinceFrameChange >= 40 && entityList[frank].ticksSinceFrameChange <= 43) {
        for (uint16_t y = 0; y < VIEWPORT_HEIGHT * TILE_IMAGE_HEIGHT; y++) {
            for (uint16_t x = 0; x < VIEWPORT_WIDTH * TILE_IMAGE_WIDTH; x++) {
                bufferBitmap[coordToOffset(x, y, screenSizeX)] = 0x00320000;
            }
        }
        return;
    }

    // loop through tile_map and display all tiles to bufferBitmap
    for (uint16_t y = 0; y < VIEWPORT_HEIGHT + 1; y++) {
        for (uint16_t x = 0; x < VIEWPORT_WIDTH + 1; x++) {

            if (camX / TILE_IMAGE_WIDTH + x >= TILE_MAP_WIDTH || camY / TILE_IMAGE_HEIGHT + y >= TILE_MAP_HEIGHT) continue;

            struct tile a = tile_map[coordToOffset(camX / TILE_IMAGE_WIDTH + x, camY / TILE_IMAGE_HEIGHT + y, TILE_MAP_WIDTH)];
            uint32_t* tileBitmap = tile_getBitmapFromID(a.tileId);
            
            for (int32_t ty = 0; ty < TILE_IMAGE_HEIGHT; ty++) {
                for (int32_t tx = 0; tx < TILE_IMAGE_WIDTH; tx++) {

                    int32_t screenX = x * TILE_IMAGE_WIDTH + tx - (camX % (TILE_IMAGE_WIDTH));
                    int32_t screenY = y * TILE_IMAGE_HEIGHT + ty - (camY % (TILE_IMAGE_HEIGHT));

                    // skip if off-screen
                    if (screenX < 0 || screenY < 0 || screenX >= VIEWPORT_WIDTH * TILE_IMAGE_WIDTH || screenY >= VIEWPORT_HEIGHT * TILE_IMAGE_HEIGHT)
                        continue;

                    bufferBitmap[coordToOffset(screenX, screenY, screenSizeX)] = tileBitmap[(ty) * TILE_IMAGE_WIDTH + (tx)];
                }
            }
        }
    }


    // loop through entities and display all entities to bufferBitmap
    for (uint32_t i = 0; i < ENTITY_MAX_ENTITIES_ALIVE; i++) {
        
        if (entityList[i].flags & ENTITY_FLAG_IS_VALID) {
            int64_t relativeEntityX = (int64_t) entityList[i].x - camX;
            int64_t relativeEntityY = (int64_t) entityList[i].y - camY;

            // add image dimensions for partial draws; if a creature is half off the screen, still display the rest of its pixels.
            // since coordinate represents the top-left of the entity, no partial draw can occur if the entity's coords are bigger than the screen
            if (relativeEntityX + entityList[i].imgX < 0 || relativeEntityX >= (TILE_IMAGE_WIDTH * VIEWPORT_WIDTH ) || relativeEntityY + entityList[i].imgY < 0 || relativeEntityY >= (TILE_IMAGE_HEIGHT * VIEWPORT_HEIGHT)) 
                continue;


            // DRAW ENTITY - IF PIXEL == 0x00FF00FF, DON'T DRAW
            uint32_t* entityImage = NULL;
            // fetch entity image. If failed, just skip entity
            if (imageLoader_fetchImage(entityList[i].image, &entityImage) != IMGLDR_ERR_SUCCESS) {
                continue;
            }

            // if entity is facing left, draw X backwards
            if (entityList[i].flags & ENTITY_FLAG_IS_FACING_LEFT) {
                for (int64_t y = 0; y < entityList[i].imgY; y++) {
                    for (int64_t x = 0; x < entityList[i].imgX; x++) {
                        if (entityImage[coordToOffset(x, y, entityList[i].imgX)] == 0x00FF00FF || relativeEntityX + x < 0 || relativeEntityX + x >= (TILE_IMAGE_WIDTH * VIEWPORT_WIDTH) || relativeEntityY + y < 0 || relativeEntityY + y >= (TILE_IMAGE_HEIGHT * VIEWPORT_HEIGHT))
                            continue;

                        bufferBitmap[coordToOffset(relativeEntityX + ((entityList[i].imgX - 1) - x), relativeEntityY + y, screenSizeX)] = entityImage[coordToOffset(x, y, entityList[i].imgX)];
                    }
                }  
            } else {
                for (int64_t y = 0; y < entityList[i].imgY; y++) {
                    for (int64_t x = 0; x < entityList[i].imgX; x++) {
                        if (entityImage[coordToOffset(x, y, entityList[i].imgX)] == 0x00FF00FF || relativeEntityX + x < 0 || relativeEntityX + x >= (TILE_IMAGE_WIDTH * VIEWPORT_WIDTH ) || relativeEntityY + y < 0 || relativeEntityY + y >= (TILE_IMAGE_HEIGHT * VIEWPORT_HEIGHT))
                            continue;

                        bufferBitmap[coordToOffset(relativeEntityX + x, relativeEntityY + y, screenSizeX)] = entityImage[coordToOffset(x, y, entityList[i].imgX)];
                    }
                }  
            }
        }
    }
}

uint32_t coordToOffset(uint16_t x, uint16_t y, uint16_t xMax) {
    return (y * xMax) + x;
}

void showLastError(const char* str) {
    DWORD errorCode = GetLastError();
    char* msgBuffer = NULL;

    FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        errorCode,
        0, // Default language
        (LPSTR) &msgBuffer,
        0,
        NULL
    );
    MessageBox(NULL, msgBuffer, str, MB_ICONWARNING);

    LocalFree(msgBuffer);
}