#ifndef __KERNEL_DEBUG_H
#define __KERNEL_DEBUG_H

void panic_spin(char* filename, int line, const char* func, char* condition);

#define PAINC(...) panic_spin(__FILE__, __LINE__, __func__,__VA_ARGS__);

#ifndef NDEBUG
    #define ASSERT(CONDITION)((void)0)
#else
#define ASSERT(CONDITION)  \
if(CONDITION){  \
    put_str("if");  \
} else {            \
    put_str("else"); \
    PAINC(#CONDITION);  \
}
#endif
#endif