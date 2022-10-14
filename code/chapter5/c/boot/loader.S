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
   times 60 dq 0					 ; 此处预留60个描述符的slot
   SELECTOR_CODE equ (0x0001<<3) + TI_GDT + RPL0         ; 相当于(CODE_DESC - GDT_BASE)/8 + TI_GDT + RPL0
   SELECTOR_DATA equ (0x0002<<3) + TI_GDT + RPL0	 ; 同上
   SELECTOR_VIDEO equ (0x0003<<3) + TI_GDT + RPL0	 ; 同上 

   ;以下是定义gdt的指针，前2字节是gdt界限，后4字节是gdt起始地址

   gdt_ptr  dw  GDT_LIMIT 
	    dd  GDT_BASE
   loadermsg db '3 loader in real.'

loader_start:


; ;----------------------------------------   准备进入保护模式   ------------------------------------------
; 									;1 打开A20
; 									;2 加载gdt
; 									;3 将cr0的pe位置1


;    ;-----------------  打开A20  ----------------
   in al,0x92
   or al,0000_0010B
   out 0x92,al

;    ;-----------------  加载GDT  ----------------
   lgdt [gdt_ptr]


;    ;-----------------  cr0第0位置1  ----------------
   mov eax, cr0
   or eax, 0x00000001
   mov cr0, eax

   jmp dword SELECTOR_CODE:p_mode_start	     ; 刷新流水线，避免分支预测的影响,这种cpu优化策略，最怕jmp跳转，
   jmp  SELECTOR_CODE:p_mode_start	     ; 刷新流水线，避免分支预测的影响,这种cpu优化策略，最怕jmp跳转，
; 					     ; 这将导致之前做的预测失效，从而起到了刷新的作用。
                  ;如果不刷新流水线，由于之前是实模式用16位驿码，因为流水线会先把后面的指令加载 那么就会出错32位的用16位的驿码

[bits 32]
p_mode_start:
   mov ax, SELECTOR_DATA
   mov ds, ax
   mov es, ax
   mov ss, ax
   mov esp,LOADER_STACK_TOP
   mov ax, SELECTOR_VIDEO
   mov gs, ax

   call setup_page
   sgdt [gdt_ptr]
   mov ebx,[gdt_ptr + 2]
   or dword [ebx + 0x18 + 4], 0xc0000000   ;gdt 使用虚拟地址 显存段
   add dword [gdt_ptr + 2], 0xc0000000  ; 
   add esp, 0xc0000000

;把页目录地址赋给 cr3
   mov eax, PAGE_DIR_TABLE_POS
   mov cr3, eax
;打开cr0的pg位(第31位)
   mov eax, cr0
   or eax, 0x80000000
   mov cr0, eax

   lgdt [gdt_ptr]



   mov byte [gs:160], 'P'

   jmp $

;------------- 创建页目录项和页表
setup_page:
;1. 清0 页目录项占用的空间
   mov ecx, 4096
   mov esi, 0
.clear_page_dir:
   mov byte [PAGE_DIR_TABLE_POS + esi], 0
   inc esi 
   loop .clear_page_dir

;创建页目录项pde
.create_pde:
   mov eax, PAGE_DIR_TABLE_POS
   add eax, 0x1000                  ;eax为第一个页表的位置
   mov ebx, eax                     ;为创建pte做准备

   or eax, PG_US_U | PG_RW_W | PG_P
   mov [PAGE_DIR_TABLE_POS + 0x0], eax       ;第一个页目录项
   mov [PAGE_DIR_TABLE_POS + 0xc00], eax     ;第768个页目录项 操作系统高1G空间映射到物理内存低1M空间
   sub eax, 0x1000
   mov [PAGE_DIR_TABLE_POS + 4092], eax      ;最后一个页目录项映射到第一个页目录项地址

   mov ecx, 256              ; 1M / 4k = 256个页表
   mov esi, 0
   mov edx, PG_US_U | PG_RW_W | PG_P         ;指向低1M内存 所以高20位都为0

;创建页目录表pte
.create_pte:
   mov [ebx + esi*4], edx
   add edx, 4096             ;下一页
   inc esi
   loop .create_pte

;创建内核其他页表的PDE
   mov eax, PAGE_DIR_TABLE_POS
   add eax, 0x2000
   or eax, PG_US_U | PG_RW_W | PG_P
   mov ebx, PAGE_DIR_TABLE_POS
   mov ecx, 254
   mov esi, 769
.create_kernel_pde:
   mov [ebx + esi*4], eax
   inc esi
   add eax, 0x1000
   loop .create_kernel_pde
   ret




