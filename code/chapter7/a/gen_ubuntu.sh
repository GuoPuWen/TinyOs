echo "--------- mbr.bin -----------------"
nasm -I boot/include/ -o ./out/mbr.bin boot/mbr.s 
echo "--------- loader.bin ---------------"
nasm -I boot/include/ -o ./out/loader.bin boot/loader.S 
echo "--------- print.o ------------------"
nasm -f elf -o out/kernel/print.o lib/kernel/print.S
echo "--------- kernel.o ------------------"
nasm -f elf  -o out/kernel/kernel.o kernel/kernel.S
echo "--------- main.o ------------------"
gcc -m32 -fno-stack-protector -I  lib/kernel/ -I lib/ -I kernel/ -c  -o   out/kernel/main.o kernel/main.c
echo "--------- interrupt.o ------------------"
gcc -m32 -fno-stack-protector -I lib/kernel -I lib/ -I kernel/ -c  -o   out/interrupt.o kernel/interrupt.c
echo "--------- init.o ------------------"
gcc -m32 -fno-stack-protector -I  lib/kernel -I lib/ -I kernel/ -c  -o   out/init.o kernel/init.c
echo "--------- ld kernel.bin ------------------"
ld -melf_i386  -Ttext 0xc0001500 -e main -o ./out/kernel/kernel.bin out/kernel/main.o out/kernel/print.o \
     out/init.o out/interrupt.o out/kernel/kernel.o 

echo "--------- kernel.bin 写入磁盘 ------------------"
