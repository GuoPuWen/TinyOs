#!/bin/bash
if [ ! -d "./out" ]; then
      mkdir out
fi
nasm -I include/ -o ./out/mbr.bin mbr.s && dd if=./out/mbr.bin of=./hd3M.img bs=512 count=1  conv=notrunc
nasm -I include/ -o ./out/loader.bin loader.S && dd if=./out/loader.bin of=./hd3M.img bs=512 count=3 seek=2 conv=notrunc
/usr/local/bin/bochs -f bochsrc