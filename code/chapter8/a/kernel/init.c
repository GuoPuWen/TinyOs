#include "init.h"
#include "print.h"
#include "interrupt.h"
#include "../device/timer.h"
#include "memory.h"

void init_all() {
    put_str("init_all \n");
    idt_init();     // 初始化idt 中断描述符
    timer_init();   //初始化PIT
    mem_init();     //初始化  内存管理
}