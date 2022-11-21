#include "memory.h"
#include "std_int.h"
#include "print.h"
#include "debug.h"
#include "string.h"

#define PAGE_SIZE 4096          //一页的大小
#define MEM_BITMAP_BASE   0xc009a000       // 页位图开始地址 总共4页 可以分配512M的内存
#define K_HEAD_START      0xc0100000         // 虚拟地址跨越低端1m

#define PDE_INX(addr) ((addr & 0xffc00000) >> 22)
#define PTE_INX(addr) ((addr & 0x003ff000) >> 12)

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
// 在pf表示的虚拟内存中申请pg_cnt个虚拟页 成功返回虚拟页的地址，失败则返NULL
void* vaddr_get(enum pool_flags pf, uint32_t pg_cnt) {
    int vaddr_start = 0, bit_idx_start = -1;
    uint32_t cnt = 0;
    if(pf == PF_KERNEL) {
        bit_idx_start = bitmap_scan(&kernel_vaddr.vaddr_bitmap, pg_cnt);
        if(bit_idx_start == -1) {
            return NULL;
        }

        while(cnt < pg_cnt) {
            bitmap_set(&kernel_vaddr.vaddr_bitmap, bit_idx_start + cnt, 1);
            cnt++;
        }
        //虚拟页的地址 bitmap中一位表示一页
        vaddr_start = kernel_vaddr.vaddr_start + bit_idx_start * PAGE_SIZE;
    }else {
        //用户内存池
    }
    return (void*)vaddr_start;
}
//得到虚拟地址vaddr对应的pte指针 同样也是虚拟地址 只不过是访问pte的虚拟地址
uint32_t* pte_ptr(uint32_t vaddr) {
    return (uint32_t*)(0xffc00000 + ((vaddr & 0xffc00000) >> 10) + PTE_INX(vaddr) * 4);
}
//得到虚拟地址vaddr对应的pde指针 同样也是虚拟地址 只不过是访问pte的虚拟地址
uint32_t* pde_ptr(uint32_t vaddr) {
    return (uint32_t*)(0xfffff000 + PDE_INX(vaddr) * 4);
}
//在m_pool指向的物理内存中分配一个物理页 成功返回页框的物理地址
static void* pmalloc(struct pool* m_pool) {
    int bit_index = bitmap_scan(&m_pool->pool_bitmap, 1);
    if(bit_index == -1) {
        return NULL;
    }
    bitmap_set(&m_pool->pool_bitmap, bit_index, 1);
    uint32_t page_phyaddr = m_pool->phy_addr_start + (bit_index * PAGE_SIZE);
    return (void*)page_phyaddr;
}

//在页表中添加虚拟地址_vaddr与物理地址_page_phyaddr 的映射
void page_table_add(void* _vaddr, void* _page_phyaddr) {
    uint32_t vaddr = (uint32_t)_vaddr;
    uint32_t page_phyaddr = (uint32_t)_page_phyaddr;
    uint32_t* pde = pde_ptr(vaddr);         //虚拟地址vaddr所在的pde的虚拟地址
    uint32_t* pte = pte_ptr(vaddr);         //虚拟地址vaddr所在的pte的虚拟地址
    //判断页目录项的p位 判断页是否存在 为1则存在
    if(*pde & 0x00000001) {
        ASSERT(!(*pde & 0x00000001));
        //页表pte应该不存在
        if(!(*pte & 0x00000001)) {
            *pte = (page_phyaddr | PG_US_U | PG_RW_W | PG_P_1);
        } else {
            PAINC("pte repeat");
        }
    //  页目录项不存在
    }else {
        //创建页目录项 分配一个物理页
        uint32_t pde_phyaddr = (uint32_t)pmalloc(&kernel_pool);
        *pde = (pde_phyaddr | PG_US_U | PG_RW_W | PG_P_1);
        //初始化pde所指向的pte的物理地址，防止pte中的陈旧数据成了页表项
        memset((void*)((int)pte & 0xfffff000), 0, PAGE_SIZE);
        ASSERT(!(*pte & 0x00000001));
        *pte = (page_phyaddr | PG_US_U | PG_RW_W | PG_P_1);
    }
}

//分配page_cnt个页空间 成功返回起始虚拟地址 失败返回NULL
void* malloc_page(enum pool_flags pf, uint32_t pg_cnt) {
    //最大分配15M空间 15*1024*1024 / 4096 = 3840页
    ASSERT(pg_cnt > 0 && pg_cnt < 3840);
    //得到虚拟地址
    void* vaddr_start = vaddr_get(pf, pg_cnt);
    if(vaddr_start == NULL) {
        return NULL;
    }
    uint32_t vaddr = (uint32_t)vaddr_start, cnt = pg_cnt;
    struct pool* mem_pool = (pf & PF_KERNEL) ? &kernel_pool : &user_pool;
    //虚拟地址是连续的 物理地址可以不连续  逐一映射
    while(cnt-- > 0) {
        void* page_phyaddr = pmalloc(mem_pool);
        if(page_phyaddr == NULL) {              //失败时需要将曾经已经申请的虚拟地址、物理页全部回滚 TODO
            return NULL;
        }
        page_table_add((void*)vaddr, page_phyaddr);
        vaddr += PAGE_SIZE;         //下一个虚拟页
    }
    return vaddr_start;
}

//从内核物理内存池中申请 pg_cnt 页内存 成功返回虚拟地址 失败返回NULL
void* get_kernel_pages(uint32_t pg_cnt) {
    void* vaddr = malloc_page(PF_KERNEL, pg_cnt);
    if(vaddr != NULL) {
        memset(vaddr, 0, pg_cnt * PAGE_SIZE);
    }
    return vaddr;
}

void mem_init() {
    put_str("mem_init start\n");
    uint32_t mem_byte_total = (*(uint32_t*)(0xb03));
    mem_pool_init(mem_byte_total);
    put_str("mem_init done\n");
}


