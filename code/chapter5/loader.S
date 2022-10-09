   %include "boot.inc"
   section loader vstart=LOADER_BASE_ADDR
   LOADER_STACK_TOP equ LOADER_BASE_ADDR
   jmp loader_start					; 此处的物理地址是:
   
;构建gdt及其内部的描述符
   GDT_BASE:   dd    0x00000000 
	       dd    0x00000000

   CODE_DESC:  dd    0x0000FFFF 
	       dd    DESC_CODE_HIGH4

   DATA_STACK_DESC:  dd    0x0000FFFF
		     dd    DESC_DATA_HIGH4

   VIDEO_DESC: dd    0x80000007	       ;limit=(0xbffff-0xb8000)/4k=0x7
	       dd    DESC_VIDEO_HIGH4  ; 此时dpl已改为0

   GDT_SIZE   equ   $ - GDT_BASE
   GDT_LIMIT   equ   GDT_SIZE -	1 
   times 60 dq 0					 ; 此处512字节
   total_mem_bytes dd 0
   ;以下是定义gdt的指针，前2字节是gdt界限，后4字节是gdt起始地址

   gdt_ptr  dw  GDT_LIMIT 
	    dd  GDT_BASE
   ards_buf times 244 db 0    ;人工对齐 共256字节
   ards-nr  dw 0           ;用于记录 ARDS 结构体数量

loader_start:
; int 15h eax = 0000E820h ,edx = 534D4150h ('SMAP') 获取内存布局
   xor ebx, ebx               ;ebx清零
   mov edx, 0x534d4150
   mov di, ards_buf
.e820_mem_get_loop:
   mov eax, 0x0000e820        ;执行 int 0x15 后，eax 值变为 0x534d4150，
   mov ecx, 20                ;ARDS 地址范围描述符结构大小是 20 字节
   int 0x15
   jc .e820_failed_so_try_e801
   add di, cx
   inc word [ards_nr]
   cmp ebx, 0
   jnz .e820_mem_get_loop

   mov cx, [ards_nr]         ;找出(base_add_low + length_low)的最大值，即内存的容量
   mov ebx, ards_buf
.find_max_mem_area:
   mov eax, [ebx]             ; base_add_low
   add eax, [ebx+8]           ; length_low
   add ebx, 20                ; 下一个结构
   cmp edx, eax               ;edx 最大内存大小
.next_ards
   loop .find_max_mem_area
   jmp .mem_get_ok

; 
