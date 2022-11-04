echo "--------- mbr.bin -----------------"
nasm -I boot/include/ -o ./out/mbr.bin boot/mbr.s 
echo "--------- loader.bin ---------------"
nasm -I boot/include/ -o ./out/loader.bin boot/loader.S 
echo "--------- print.o ------------------"
nasm -f elf -o out/kernel/print.o lib/kernel/print.S
echo "--------- kernel.o ------------------"
nasm -f elf  -o out/kernel/kernel.o kernel/kernel.S

gcc -I lib/kernel/ -I lib/ -I kernel/ -c -fno-builtin -m32 -fno-stack-protector -o out/main.o kernel/main.c 
gcc -I lib/kernel/ -I lib/ -I kernel/ -c -fno-builtin -m32 -fno-stack-protector -o out/interrupt.o kernel/interrupt.c 
gcc -I lib/kernel/ -I lib/ -I kernel/ -I device/ -c -fno-builtin -m32 -fno-stack-protector -o out/init.o kernel/init.c 
gcc -I lib/kernel/ -I lib/ -c -m32 -o out/timer.o device/timer.c
echo "--------- ld kernel.bin ------------------"
ld -melf_i386  -Ttext 0xc0001500 -e main -o ./out/kernel/kernel.bin  out/kernel/main.o out/kernel/print.o \
     out/init.o out/interrupt.o out/kernel/kernel.o   out/timer.o

echo "--------- kernel.bin 写入磁盘 ------------------"
