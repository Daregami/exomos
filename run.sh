#!/bin/bash

nasm -f bin boot/boot.asm -i boot/ -o build/boot.bin
nasm -f bin boot/loader.asm -i boot/ -o build/loader.bin

dd if=/dev/zero of=build/kernel bs=2M count=500

cat build/boot.bin build/loader.bin build/kernel > build/exomos.bin

qemu-system-i386 -drive format=raw,file=build/exomos.bin
