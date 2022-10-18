TI_GDT      equ     0
RPL0        equ     0
SELECTOR_VIDEO  equ (0x0003 << 3) + TI_GDT + RPL0

[bits 32]
section .text
; 功能描述:把栈中的1个字符写入光标所在处
global put_char
put_char:
    pushad                      ;备份32位寄存器环境
    mov ax, SELECTOR_VIDEO      ;为gs赋值   
    mov gs, ax              

;获取当前光标位置
    ;高8位  
    mov dx, 0x03d4              ;索引寄存器
    mov al, 0x0e                ;光标的高8位
    out dx, al
    mov dx, 0x03d5              ;通过读写数据端口0x3d5来获得或设置光标位置
    in  al, dx                  ;得到了光标位置的高8位
    mov ah, al
    ;低8位  
    mov dx, 0x03d4
    mov al, 0x0f
    out dx, al
    mov dx, 0x03d5
    in  al, dx              

    mov bx, ax                  ;将光标存入bx
    
    mov ecx, [esp + 36]         ;在栈中获取待打印的字符 pushad一共4*8=32 + 返回地址4字节
    cmp cl, 0xd                 ;CR是0x0d,LF是0x0a 回车换行
    jz  .is_carriage_return
    cmp cl, 0xa    
    jz  .is_line_feed    
    cmp cl, 0x8                 ;baskspace
    jz .is_backspace
    jmp .put_other     
    
;处理退格
.is_backspace:
    dec bx                      ;光标退一个格子
    shl bx, 1                   ;取得显存地址 插入空格
    mov byte [gs:bx], 0x20      ;空格
    inc bx
    mov byte [gs:bx], 0x07
    shr bx, 1                   ;复原光标
    jmp .set_cursor

.put_other:
    shl bx, 1                   ;获取显存地址
    mov [gs:bx], cl             ;插入字符
    inc bx  
    mov byte [gs:bx], 0x07      ;字符属性
    shr bx, 1
    inc bx                      ;下一个光标
    cmp bx, 2000    
    jl  .set_cursor             ;光标小于2000

.is_line_feed:                  ;是换行符LF(\n)
.is_carriage_return:            ;是回车符CR(\r)    取得行首光标 当前光标/80 得到余数 
                                ;当前光标 - 余数 则是行首
    xor dx, dx                  ; dx是被除数的高16位,清0.
    mov ax, bx                  ;当前光标
    mov si, 80
    div si
    sub bx, dx

 .is_carriage_return_end:
    add bx, 80
    cmp bx, 2000
 .is_line_feed_end:
    jl .set_cursor


;屏幕行范围是0~24,滚屏的原理是将屏幕的1~24行搬运到0~23行,再将第24行用空格填充
.roll_screen:
    cld                         ;movsd三件套
    mov ecx, 960                ; 一共有2000-80=1920个字符要搬运,共1920*2=3840字节.一次搬4字节,共3840/4=960次 
    mov esi, 0xb80a0            ;第1行行首
    mov edi, 0xb8000            ;第0行行首
    rep movsb
;将最后一行填充为空白
    mov ebx, 3840               ;最后一行首字符的第一个字节偏移= 1920 * 2
    mov ecx, 80                 ;一行是80字符(160字节),每次清空1字符(2字节),一行需要移动80次
 .cls:
    mov word [gs:ebx], 0x0720
    add ebx, 2
    loop .cls
    mov bx, 1920                ;将光标值重置为1920,最后一行的首字符.

.set_cursor:
    ; 1 先设置高8位
    mov dx, 0x03d4
    mov al, 0x0e
    out dx, al
    mov dx, 0x03d5
    mov al, bh
    out dx, al
    
    ;再设置低8位
    mov dx, 0x03d4
    mov al, 0x0f
    out dx, al
    mov dx, 0x03d5 
    mov al, bl
    out dx, al
 .put_char_done: 
   popad
   ret








