#include "memory.h"
#include "std_int.h"
#include "print.h"

#define PAGE_SIZE 4096          //一页的大小
#define MEM_BITMAP_BASE   0xc009a000       // 页位图开始地址 总共4页 可以分配512M的内存
#define K_HEAD_START      0xc0100000         // 虚拟地址跨越低端1m
struct pool {
    struct bitmap pool_bitmap;
    uint32_t phy_addr_start;            //物理内存的起始地址
    uint32_t pool_size;
};

struct pool kernel_pool, user_pool;     //内核物理内存池 用户物理内存池
struct virtual_addr kernel_vaddr;        //内核虚拟地址内存池

static void mem_pool_init(uint32_t all_men) {
    put_str("mem_pool_init start\n");
    // 页目录本身1个 769～1022 共254个 0和768指向同低端1m占1个 所以256个
    uint32_t page_table_size = PAGE_SIZE * 256;     //记录页目录表和页表占用的字节大小
    uint32_t used_mem = page_table_size + 0x100000; //已经使用的地址
    uint32_t free_mem = all_men - used_mem;         // 空闲内存
    uint16_t all_free_pages = free_mem / PAGE_SIZE;  //总页数 不足1页的不考虑
    uint16_t kernel_free_pages = all_free_pages / 2;        //内核占用的页数
    uint16_t user_free_pages = all_free_pages - kernel_free_pages;  //用户占用的页数
    
    uint32_t kbm_length = kernel_free_pages / 8;            // 内核bitmap大小 1位表示1页 不足的不表示
    uint32_t ubm_length = user_free_pages / 8;

    uint32_t kp_start = used_mem;               // 内核内存起始地址
    uint32_t up_start = kp_start + kernel_free_pages * PAGE_SIZE;   //用户内存起始地址

    kernel_pool.phy_addr_start = kp_start;
    user_pool.phy_addr_start = up_start;

    kernel_pool.pool_size = kernel_free_pages * PAGE_SIZE;
    user_pool.pool_size = user_free_pages * PAGE_SIZE;

    kernel_pool.pool_bitmap.btmap_byte_len = kbm_length;
    user_pool.pool_bitmap.btmap_byte_len = ubm_length;

    kernel_pool.pool_bitmap.bits = (void*)MEM_BITMAP_BASE;
    user_pool.pool_bitmap.bits = (void*)(MEM_BITMAP_BASE + kbm_length);


    put_str("kernel_pool_bitmap_start:");
    put_int((int)kernel_pool.pool_bitmap.bits);
    put_str("   kernel_pool_phy_addr_start:");
    put_int(kernel_pool.phy_addr_start);
    put_str("\n");
    put_str("user_pool_bitmap_start:");
    put_int((int)user_pool.pool_bitmap.bits);
    put_str("    user_pool_phy_addr_start:");
    put_int(user_pool.phy_addr_start);
    put_str("\n");

    //  初始化位图
    bitmap_init(&kernel_pool.pool_bitmap);
    bitmap_init(&user_pool.pool_bitmap);


    //  初始化内核虚拟地址位图 
    kernel_vaddr.vaddr_bitmap.btmap_byte_len = kbm_length;
    kernel_vaddr.vaddr_bitmap.bits = (void*)(MEM_BITMAP_BASE + kbm_length + ubm_length);
    kernel_vaddr.vaddr_start = K_HEAD_START;

    bitmap_init(&kernel_vaddr.vaddr_bitmap);
    put_str("mem_pool_init done\n");

}
void mem_init() {
    put_str("mem_init start\n");
    uint32_t mem_byte_total = (*(uint32_t*)(0xb03));
    mem_pool_init(mem_byte_total);
    put_str("mem_init done\n");
}

