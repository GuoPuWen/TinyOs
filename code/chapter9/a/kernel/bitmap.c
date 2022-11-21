#include "bitmap.h"
#include "std_int.h"
#include "string.h"
#include "print.h"
#include "interrupt.h"
#include "debug.h"


void bitmap_init(struct bitmap* bmp) {
    memset(bmp->bits, 0 , bmp->btmap_byte_len);
}

//判断bit_idx位是否为1，若为1则返回true 
bool bitmap_scan_test(struct bitmap* bmp, uint32_t bit_idx) {
    uint32_t byte_byte = bit_idx / 8;
    uint32_t bit_odd = bit_idx % 8;
    return (bmp->bits[byte_byte] & (BITMAP_MASK << bit_odd));
}
//在位图中申请连续cnt个位
int bitmap_scan(struct bitmap* bmp, uint32_t cnt ) {
    //找到当前空闲位置 索引
    uint32_t idx_free_byte = 0;
    //0xff表示没有空闲位
    while((0xff == bmp->bits[idx_free_byte]) && (idx_free_byte < bmp->btmap_byte_len)) {
        idx_free_byte++;
    }
    ASSERT(idx_free_byte < bmp->btmap_byte_len);
    if(idx_free_byte == bmp->btmap_byte_len) return -1;
    //找到当前空闲位置 位
    int idx_free_bit = 0;
    while((uint8_t)(BITMAP_MASK << idx_free_bit) & bmp->bits[idx_free_byte]) {
        idx_free_bit++;         //idx_free_bit + 1 为空闲
    } 
    int bit_idx_start = idx_free_byte * 8 + idx_free_bit;       // 空闲位在位图内的下标
    if(cnt == 1) {
        return bit_idx_start;
    }
    uint32_t bit_left = (bmp->btmap_byte_len * 8 - bit_idx_start);
    uint32_t next_bit = bit_idx_start + 1;                      // 用于记录找到的空闲位的个数

    bit_idx_start = -1;
    uint32_t count = 1;
    while(bit_left-- > 0) {
        if(!(bitmap_scan_test(bmp, next_bit))) {
            count++;
        }else {
            count = 0;                          //被占用 重0开始
        }
        if(count == cnt) {
            bit_idx_start = next_bit - cnt + 1; // cnt = next_bit - bit_idx_start + 1;
            break;
        }
        next_bit++;
    }
    return bit_idx_start;
}
//将位图 btmp 的 bit_idx 位设置为 value
void bitmap_set(struct bitmap* bmp, uint32_t bit_idx, int8_t value) {
    ASSERT((value == 1) || (value == 0));
    uint32_t byte_idx = bit_idx / 8;
    uint8_t bit_odd = bit_idx % 8;
    if(value) {
        bmp->bits[byte_idx] |= (BITMAP_MASK << bit_odd);
    }else {
        bmp->bits[byte_idx] &= ~(BITMAP_MASK << bit_odd);
    }
}
