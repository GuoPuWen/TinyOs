echo "--------- mbr.bin -----------------"
nasm -I boot/include/ -o ./out/mbr.bin boot/mbr.s 
echo "--------- loader.bin ---------------"
nasm -I boot/include/ -o ./out/loader.bin boot/loader.S 
echo "--------- print.o ------------------"
nasm -f elf -o out/kernel/print.o lib/kernel/print.S
echo "--------- main.o ------------------"
gcc -m32 -I lib/kernel/ -I lib/ -c -o out/kernel/main.o kernel/main.c
echo "--------- kernel.bin ------------------"
ld -melf_i386  -Ttext 0xc0001500 -e main -o ./out/kernel/kernel.bin out/kernel/main.o out/kernel/print.o
echo "--------- kernel.bin 写入磁盘 ------------------"
