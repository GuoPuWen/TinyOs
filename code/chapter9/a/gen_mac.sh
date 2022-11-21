dd if=./out/mbr.bin of=./hd3M.img bs=512 count=1  conv=notrunc
dd if=./out/loader.bin of=./hd3M.img bs=512 count=3 seek=2 conv=notrunc
dd if=./out/kernel/kernel.bin of=./hd3M.img bs=512 count=200 seek=9 conv=notrunc