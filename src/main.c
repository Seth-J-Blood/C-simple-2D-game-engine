#include "tiles.h"

// GLOBAL VARIABLES //
HWND winHandle;             // a handle to the window associated with this program
HDC bufferHdc;              // a device context used to buffer the bitmap updates
HBITMAP bufferHandle;       // a handle to the bitmap used to buffer screen updates
const RECT dispDimensions = {
    .left = 0,
    .top = 0,
    .right = TILE_IMAGE_WIDTH * TILE_MAP_WIDTH,
    .bottom = TILE_IMAGE_HEIGHT * TILE_MAP_HEIGHT
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

uint16_t dispSizeX = 0;         // how wide the window is, in pixels
uint16_t dispSizeY = 0;         // how tall the window is, in pixels
uint16_t screenSizeX = 0;       // how big the maximum width is, in pixels
uint16_t screenSizeY = 0;       // how big the maximum height is, in pixels
uint32_t* bufferBitmap = NULL;  // an array that contains the values of the buffer bitmap
uint32_t isStarted = 0;

// FUNCTION PROTOTYPES //
void cleanup();
void refreshScreen();
uint32_t coordToOffset(uint16_t x, uint16_t y, uint16_t xMax);
void showLastError(const char*);

LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_SIZE: {
            // handle resizing //
        }
        return 0;
        case WM_PAINT: {
            refreshScreen();

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
        TILE_MAP_WIDTH * TILE_IMAGE_WIDTH,
        TILE_MAP_HEIGHT * TILE_IMAGE_HEIGHT + GetSystemMetrics(SM_CYCAPTION),
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
    
    // INITIALIZE TILES
    if (tile_init(bufferHdc, TILE_MAP_WIDTH, TILE_MAP_HEIGHT) != TILE_ERR_SUCCESS) {
        MessageBox(NULL, "Error: tile_init failed!", "Debug box", MB_ICONWARNING);
        cleanup();
        return -1;
    }
    isStarted = 1;
    if (tileBitmapInfos[0].pixels == NULL) {
        MessageBox(NULL, "Error: tileBitMapInfo didn't initialize!", "Debug box", MB_ICONWARNING);
        cleanup();
        return -1;
    }
    
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

    // loop through tile_map and display all tiles to bufferBitmap
    for (uint16_t y = 0; y < TILE_MAP_HEIGHT; y++) {
        for (uint16_t x = 0; x < TILE_MAP_WIDTH; x++) {

            struct tile a = tile_map[coordToOffset(x, y, TILE_MAP_WIDTH)];
            uint32_t* tileBitmap = tile_getBitmapFromID(1);

            for (int i = 0; i < TILE_IMAGE_HEIGHT * TILE_IMAGE_WIDTH; i++) {
                bufferBitmap[coordToOffset(x * TILE_IMAGE_WIDTH + (i % TILE_IMAGE_WIDTH), y * TILE_IMAGE_HEIGHT + (i / TILE_IMAGE_WIDTH), screenSizeX)] = tileBitmap[i];
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