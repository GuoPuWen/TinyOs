   %include "boot.inc"
   section loader vstart=LOADER_BASE_ADDR
   LOADER_STACK_TOP equ LOADER_BASE_ADDR
   jmp loader_start
   
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
      SELECTOR_CODE equ (0x0001<<3) + TI_GDT + RPL0         ; 相当于(CODE_DESC - GDT_BASE)/8 + TI_GDT + RPL0
   SELECTOR_DATA equ (0x0002<<3) + TI_GDT + RPL0	 ; 同上
   SELECTOR_VIDEO equ (0x0003<<3) + TI_GDT + RPL0	 ; 同上 
   total_mem_bytes dd 0
   ;以下是定义gdt的指针，前2字节是gdt界限，后4字节是gdt起始地址

   gdt_ptr  dw  GDT_LIMIT 
	    dd  GDT_BASE
   ards_buf times 244 db 0    ;人工对齐 共256字节
   ards_nr  dw 0           ;用于记录 ARDS 结构体数量

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
   xor edx, edx
.find_max_mem_area:
   mov eax, [ebx]             ; base_add_low
   add eax, [ebx+8]           ; length_low
   add ebx, 20                ; 下一个结构
   cmp edx, eax               ;edx 最大内存大小
   jge .next_ards
   mov edx, eax
.next_ards:
   loop .find_max_mem_area
   jmp .mem_get_ok

; int 15h ax = E801h 获取内存大小，最大支持 4G
; 返回后, ax cx 值一样,以 KB 为单位，bx dx 值一样，以 64KB 为单位 
; 在ax和cx寄存器中为低16MB，在bx和dx寄存器中为16MB到4GB

.e820_failed_so_try_e801:
   mov ax, 0xe801
   int 0x15
   jc .e801_failed_so_try88

;第一步 计算低15M的内存
   mov cx, 0x400            ;1kb 
   mul cx                  ;16位乘法被乘数在ax 积高16位DX 低16位AX
   shl edx, 16
   and eax, 0x0000FFFF
   or edx, eax             ;最终结果在edx
   add edx, 0x100000       ;加上1M 不计数的1M
   mov esi, edx            ;备份
;第二步 加上16M以上
   xor eax, eax
   mov ax, bx
   mov ecx, 0x10000        ;64kb 单位
   mul ecx                 ;32位乘法 被乘数在eax 高32位edx 低32位eax
   add esi, eax            ;内存最多32位 所以只有eax
   mov edx, esi
   jmp .mem_get_ok

;----- int 15h ah = 0x88 获取内存大小，只能获取 64MB 之内 -----
.e801_failed_so_try88:
   mov ah, 0x88
   int 0x15
   jc .error_hlt
   and eax, 0x0000FFFF
   mov cx, 0x400
   mul cx
   shl edx, 16
   or edx, eax
   add edx, 0x100000

.mem_get_ok:
   mov [total_mem_bytes], edx	 ;将内存换为byte单位后存入total_mem_bytes处。
;-----------------   准备进入保护模式   -------------------
;1 打开A20
;2 加载gdt
;3 将cr0的pe位置1

   ;-----------------  打开A20  ----------------
   in al,0x92
   or al,0000_0010B
   out 0x92,al

   ;-----------------  加载GDT  ----------------
   lgdt [gdt_ptr]

   ;-----------------  cr0第0位置1  ----------------
   mov eax, cr0
   or eax, 0x00000001
   mov cr0, eax

   jmp dword SELECTOR_CODE:p_mode_start	     ; 刷新流水线，避免分支预测的影响,这种cpu优化策略，最怕jmp跳转，
					     ; 这将导致之前做的预测失效，从而起到了刷新的作用。
.error_hlt:		      ;出错则挂起
   hlt

[bits 32]
p_mode_start:
   mov ax, SELECTOR_DATA
   mov ds, ax
   mov es, ax
   mov ss, ax
   mov esp,LOADER_STACK_TOP
   mov ax, SELECTOR_VIDEO
   mov gs, ax

   mov byte [gs:160], 'P'

   jmp $