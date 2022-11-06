#ifndef __LIB_KERNEL_BITMAP_H
#define __LIB_KERNEL_BITMAP_H
#include "global.h"
#define BITMAP_MASK 1
struct bitmap { 
    uint32_t btmap_byte_len;
    uint8_t* bits; //在遍历位图时，整体上以字节为单位，细节上是以位为单位
};
#endif