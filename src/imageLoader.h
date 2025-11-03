#ifndef __IMAGELOADER_INCLUDE_GUARD__
#define __IMAGELOADER_INCLUDE_GUARD__
#include <windows.h>        // windows functionality
#include <stdint.h>         // standard int data types 
#include <malloc.h>         // memory allocation/copying

#ifndef IMGLDR_MAX_IMAGES_DEFINED
#define IMGLDR_MAX_IMAGES_DEFINED               512     // MUST BE A MULTIPLE OF 8
#endif  

#define IMGLDR_INVALID_IMAGE                    -1

#define IMGLDR_ERR_SUCCESS                      0
#define IMGLDR_ERR_INIT_DIRECTORY_FAILED        1
#define IMGLDR_ERR_IMAGE_NOT_LOADED             2
#define IMGLDR_ERR_NOT_INITIALIZED              3
#define IMGLDR_ERR_ALREADY_INITIALIZED          4
#define IMGLDR_ERR_DIRECTORY_DAMAGED            5
#define IMGLDR_ERR_NO_MEMORY                    6
#define IMGLDR_ERR_MAX_IMAGES_DEFINED           7
#define IMGLDR_ERR_BAD_PARAMETERS               8

typedef uint32_t imageHandle_t;

// initializes the image loader function
uint32_t imageloader_init(HDC hdc);
uint32_t imageLoader_close();
imageHandle_t imageLoader_newImage(const PTCHAR absolute_filename, uint16_t imageWidth, uint16_t imageHeight);     // requires absolute filepath from root!
imageHandle_t imageLoader_newImage_output(const PTCHAR absolute_filename, uint16_t imageWidth, uint16_t imageHeight, uint32_t** output);     // requires absolute filepath from root!
imageHandle_t imageLoader_fetchImage(imageHandle_t image, uint32_t** output);
uint32_t imageLoader_deleteImage(imageHandle_t image);
uint32_t imageLoader_getLastError();

#endif