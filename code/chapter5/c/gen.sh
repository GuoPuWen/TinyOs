gcc -c -o out/kernel/main.o kernel/main.c 
ld out/kernel/main.o -Ttext 0xc0001500 -e main -o out/kernel/kernel.bin 
dd if=out/kernel/kernel.bin  of=hd3M.img bs=512 count=200 seek=9 conv=notrunc