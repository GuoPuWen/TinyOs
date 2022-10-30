[bits 32]
%define ERROR_CODE  nop                 ; 代表为空
%define ZERO       push 0                   ; 保持栈格式一致

extern put_str                          ; 外部函数 打印字符串

extern idt_table                         ;引用外部 中断处理程序数组

section .data
intr_str db "interrupt occur!", 0xa, 0
global intr_entry_table                ; 中断数组 声明为外部
intr_entry_table:

%macro VECTOR 2                      ; 定义多行宏
section .text
intr%1entry:
    %2                                 ; 是否压入错误码
    ;保存上下文 后面调用函数
    push ds
    push es
    push fs
    push gs
    pushad                              ; EAX,ECX,EDX,EBX,ESP,EBP,ESI,EDI,EAX最先入栈

    ; 结束中断信号
    mov al, 0x20                        ;中断结束命令 EOI          
    out 0xa0, al                        ;向从片发送
    out 0x20, al                        ;向主片发送

    push %1                             ;压入中断向量号
    call [idt_table + %1 * 4]           ;一个函数地址占字节
    jmp intr_exit

section .data  
    dd intr%1entry                      ;存储各个中断程序入口 nasm编译时会将相同段合并
%endmacro

section .text
global intr_exit
intr_exit:
    add esp, 4                         ; 跳过中断号
    popad
    pop gs
    pop fs
    pop es
    pop ds
    add esp, 4                          ; 跳过错误码
    iretd

;定义33个中断
VECTOR 0x00,ZERO
VECTOR 0x01,ZERO
VECTOR 0x02,ZERO
VECTOR 0x03,ZERO 
VECTOR 0x04,ZERO
VECTOR 0x05,ZERO
VECTOR 0x06,ZERO
VECTOR 0x07,ZERO 
VECTOR 0x08,ERROR_CODE
VECTOR 0x09,ZERO
VECTOR 0x0a,ERROR_CODE
VECTOR 0x0b,ERROR_CODE 
VECTOR 0x0c,ZERO
VECTOR 0x0d,ERROR_CODE
VECTOR 0x0e,ERROR_CODE
VECTOR 0x0f,ZERO 
VECTOR 0x10,ZERO
VECTOR 0x11,ERROR_CODE
VECTOR 0x12,ZERO
VECTOR 0x13,ZERO 
VECTOR 0x14,ZERO
VECTOR 0x15,ZERO
VECTOR 0x16,ZERO
VECTOR 0x17,ZERO 
VECTOR 0x18,ERROR_CODE
VECTOR 0x19,ZERO
VECTOR 0x1a,ERROR_CODE
VECTOR 0x1b,ERROR_CODE 
VECTOR 0x1c,ZERO
VECTOR 0x1d,ERROR_CODE
VECTOR 0x1e,ERROR_CODE
VECTOR 0x1f,ZERO 
VECTOR 0x20,ZERO