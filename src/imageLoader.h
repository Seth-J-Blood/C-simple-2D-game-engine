#ifndef __IMAGELOADER_INCLUDE_GUARD__
#define __IMAGELOADER_INCLUDE_GUARD__
#include <windows.h>        // windows functionality
#include <stdint.h>         // standard int data types 
#include <malloc.h>         // memory allocation/copying

#ifndef IMGLDR_MAX_IMAGES_DEFINED
#define IMGLDR_MAX_IMAGES_DEFINED               512
#endif

#define IMGLDR_ERR_SUCCESS                      0
#define IMGLDR_ERR_INIT_DIRECTORY_FAILED        1
#define IMGLDR_ERR_INIT_IMAGE_LOAD_FAILED       2
#define IMGLDR_ERR_NOT_INITIALIZED              3
#define IMGLDR_ERR_ALREADY_INITIALIZED          4
#define IMGLDR_ERR_DIRECTORY_DAMAGED            5
#define IMGLDR_ERR_NO_MEMORY                    6
#define IMGLDR_ERR_MAX_IMAGES_DEFINED           7

// initializes the image loader function
uint32_t imageloader_init(HDC hdc);


#endif