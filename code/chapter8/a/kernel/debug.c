#include "debug.h"
#include "print.h"
#include "interrupt.h"

void panic_spin(char* filename,          \
                int line ,               \
                const char* func ,       \
                char* condition)   \

{
    intr_disable();                 //关中断
    put_str("\n\n\n!!!! error !!!!\n\n\n");
    put_str("filename:");put_str(filename);put_str("\n");
    put_str("line:0x");put_int(line);put_str("\n");
    put_str("function:");put_str((char*)func);put_str("\n");
    put_str("condition: ");put_str(condition);put_char('\n');
    while(1);
}