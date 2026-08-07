#!/bin/bash

nasm -f bin boot/boot.asm -i boot/ -o build/boot.bin
nasm -f bin boot/loader.asm -i boot/ -o build/loader.bin

# dd if=/dev/zero of=build/kernel bs=2M count=500
# printf "ELF123" | dd of=build/kernel bs=1 count=6 conv=notrunc

# Переключаем компилятор в 32-битный режим без всяких зависимостей от библиотек glibc
# -c только скомпилировать не в объектный файл
gcc -m32 -ffreestanding -fno-pic -O2 -c kernel/kernel.c -o build/kernel.o
gcc -m32 -ffreestanding -fno-pic -O2 -c kernel/pic.c -o build/pic.o
gcc -m32 -ffreestanding -fno-pic -O2 -c drivers/vga.c -o build/vga.o
gcc -m32 -ffreestanding -fno-pic -O2 -c kernel/idt.c -o build/idt.o
gcc -m32 -ffreestanding -fno-pic -O2 -c kernel/keyboard.c -o build/keyboard.o
gcc -m32 -ffreestanding -fno-pic -O2 -c kernel/alloc.c -o build/alloc.o
nasm -f elf32 kernel/isr.asm -o build/isr.o

# Указываем формат нужной архитектуры, --oformat binary выбрасывает служебную информацию и заголовок
# ld -m elf_i386 -T kernel/kernel.ld --oformat binary build/*.o -o build/kernel.bin
ld -m elf_i386 -T kernel/kernel.ld --oformat binary build/kernel.o build/pic.o build/vga.o build/idt.o build/isr.o build/keyboard.o build/alloc.o -o build/kernel.bin

cat build/boot.bin build/loader.bin build/kernel.bin > build/exomos.bin

# Расширяем финальный образ до 20 Мегабайт
truncate -s 20M build/exomos.bin

# qemu-system-i386 -drive format=raw,file=build/exomos.bin
qemu-system-i386 -drive format=raw,file=build/exomos.bin -audiodev pa,id=snd0 -machine pcspk-audiodev=snd0
