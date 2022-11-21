#ifndef  __LIB_KERNEL_MEMORY_H
#define __LIB_KERNEL_MEMORY_H
#include "std_int.h"
#include "bitmap.h"

// 用于虚拟地址池 管理虚拟地址
struct virtual_addr {
    struct bitmap vaddr_bitmap;         //虚拟地址用到的位图结构
    uint32_t vaddr_start;               //虚拟地址起始结构
};

enum pool_flags {
    PF_KERNEL = 1,      //内核内存池
    PF_USER = 2         //用户内存池
};

#define PG_P_1      1       //页表项或者页目录项存在属性位
#define PG_P_0      0        //页表项或者页目录项存在属性位
#define PG_RW_R     0       //  R/W属性位 
#define PG_RW_W     2       // 读/写/执行
#define PG_US_S     0       // U/S属性位 系统级
#define PG_US_U     4       // U/S属性位 用户级

// extern struct poll kernel_pool, user_pool;
void mem_init(void);
void* vaddr_get(enum pool_flags pf, uint32_t pg_cnt);
uint32_t* pte_ptr(uint32_t vaddr);
uint32_t* pde_ptr(uint32_t vaddr);
void page_table_add(void* _vaddr, void* _page_phyaddr);
void* malloc_page(enum pool_flags pf, uint32_t pg_cnt);
void* get_kernel_pages(uint32_t pg_cnt);
#endif