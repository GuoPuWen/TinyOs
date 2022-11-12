#ifndef  __LIB_KERNEL_MEMORY_H
#define __LIB_KERNEL_MEMORY_H
#include "std_int.h"
#include "bitmap.h"

// 用于虚拟地址池 管理虚拟地址
struct virtual_addr {
    struct bitmap vaddr_bitmap;         //虚拟地址用到的位图结构
    uint32_t vaddr_start;               //虚拟地址起始结构
};

// extern struct poll kernel_pool, user_pool;
void mem_init(void);
#endif