#!/bin/bash
if [ ! -d "./out" ]; then
      mkdir out
fi
nasm -I boot/include/ -o ./out/mbr.bin boot/mbr.s && dd if=./out/mbr.bin of=./hd3M.img bs=512 count=1  conv=notrunc
nasm -I boot/include/ -o ./out/loader.bin boot/loader.S && dd if=./out/loader.bin of=./hd3M.img bs=512 count=3 seek=2 conv=notrunc
/usr/local/bin/bochs -f bochsrc