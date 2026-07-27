#!/bin/bash

nasm -f bin boot/boot.asm -i boot/ -o build/boot.bin
nasm -f bin boot/loader.asm -i boot/ -o build/loader.bin

cat build/boot.bin build/loader.bin > build/exomos.bin

qemu-system-i386 -drive format=raw,file=build/exomos.bin
