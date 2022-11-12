#include "string.h"
#include "global.h"
#include "debug.h"

void memset(void* dst_, uint8_t value, uint32_t size) {
    ASSERT(dst_ != NULL);
    uint8_t* dst = (uint8_t*)dst_;
    while(size-- > 0) {
        *dst++ = value;
    }
}
void memcpy(void* dst_, const void* src_, uint32_t size) {
    ASSERT(dst_ != NULL && src_ != NULL);
    uint8_t *dst = (uint8_t*)dst_;
    const uint8_t* src = (uint8_t*)src_;
    while(size-- > 0) {
        *dst++ = *src++;
    }
}
//连续比较以地址a和地址b开始的size个字节
int memcmp(const void* a_, const void* b_, uint32_t size) {
    const char* a = a_;
    const char* b = b_;
    while(size-- > 0) {
        if(*a != *b) {
            return *a > *b ? 1 : -1;
        }
        *a++;
        *b++;
    }
    return 0;
}
//将字符串从srt复制到dst
const char* strcpy(char* dst_, const char* src_) {
    ASSERT(dst_ != NULL && src_ != NULL);
    const char *r = src_;
    while((*dst_++ = *src_ ++));
    return r;
}
//返回字符串的长度
uint32_t strlen(const char* str) {
    ASSERT(str != NULL);
    const char*p = str;
    while(*p++);
    return p - str - 1;
}
//比较两个字符串 a大于b返回1 相等返回0 小于返回-1
int8_t strcmp(const char* a, const char* b) {
    ASSERT(a != NULL && b != NULL);
    while(*a != 0 && *a == *b) {
        a++;
        b++;
    }
    return *a < *b ? -1 : *a > *b;
}
//从左到右查找字符串 str 中首次出现字符 ch 的地址
char* strchr(const char* str, const uint8_t ch) {
    ASSERT(str != NULL);
    while(*str != 0) {
        if(*str == ch) {
            return (char*)str;
        }
        str++;
    }
    return NULL;
}
// 从后往前查找字符串 str 中首次出现字符 ch 的地址
//从头到尾遍历一次，若存在 ch 字符，last_char 总是该字符最后一次
char* strrchr(const char* str, const uint8_t ch) {
    ASSERT(str != NULL);
    const char* last_char = NULL;
    while(*str != 0) {
        if(*str == ch) {
            last_char = str;
        }
        str++;
    }
    return (char*)last_char;
}
// 将字符串 src_拼接到 dst_后，返回拼接的串地址
char* strcat(char* dst_, const char* src_) {
    ASSERT(str != NULL);
    char* str = dst_;
    while(*str++);
    --str;
    while(*str++ = *src_++);        // 正好添加了字符串结尾的 0
    return dst_;
}
// 在字符串 str 中查找字符 ch 出现的次数
uint32_t strchrs(const char* str, uint8_t ch) {
    ASSERT(str != NULL);
    uint32_t ch_cnt = 0;
    const char* p = str;
    while(*p++) {
        if(*p == ch) {
            ch_cnt++;
        }
    }
    return ch_cnt;
}