#include "init.h"
#include "print.h"
#include "interrupt.h"
#include "../device/timer.h"

void init_all() {
    put_str("init_all \n");
    idt_init();     // 初始化idt 中断描述符
    // timer_init();   //初始化PIT
}