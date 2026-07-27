#!/bin/bash

nasm -f bin boot/boot.asm -i boot/ -o build/boot.bin

qemu-system-i386 -drive format=raw,file=build/boot.bin
