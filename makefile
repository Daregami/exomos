CC = gcc # Команда компилятора
CFLAGS = -m32 -ffreestanding -fno-pic -O2 -c
# Переключаем компилятор в 32-битный режим без всяких зависимостей от библиотек glibc
# -c только скомпилировать не в объектный файл
LD = ld # Линковщик объектных файлов
LDFLAGS = -m elf_i386 -T kernel/kernel.ld --oformat binary # Указываем формат нужной архитектуры, --oformat binary выбрасывает служебную информацию и заголовок
NASM = nasm # Компилятор ассемблера
QEMU = qemu-system-i386 # Эмулятор для запуска ОС

# Создаем список всех сишных файлы кроме kernel.c
C_SOURCES = $(filter-out kernel/kernel.c, $(wildcard kernel/*.c) $(wildcard drivers/*.c))
C_OBJECTS = $(patsubst %.c, build/%.o, $(notdir $(C_SOURCES)))

# kernel.o всегда первый при линковке
OBJECTS = build/kernel.o $(C_OBJECTS) build/isr.o

IMAGE = build/exomos.bin

# Главная цель сборки
all: $(IMAGE)

# Будет выполнять цели, пока не соберет полностью ядро
build/boot.bin: boot/boot.asm
	$(NASM) -f bin $< -i boot/ -o $@

build/loader.bin: boot/loader.asm
	$(NASM) -f bin $< -i boot/ -o $@

build/kernel.o: kernel/kernel.c
	$(CC) $(CFLAGS) $< -o $@

build/%.o: kernel/%.c
	$(CC) $(CFLAGS) $< -o $@

build/%.o: drivers/%.c
	$(CC) $(CFLAGS) $< -o $@

build/isr.o: kernel/isr.asm
	$(NASM) -f elf32 $< -o $@

build/kernel.bin: $(OBJECTS)
	$(LD) $(LDFLAGS) $(OBJECTS) -o $@

$(IMAGE): build/boot.bin build/loader.bin build/kernel.bin
	cat $^ > $@
	truncate -s 20M $@

run: $(IMAGE)
	$(QEMU) -drive format=raw,file=$(IMAGE) -audiodev pa,id=snd0 -machine pcspk-audiodev=snd0

clean:
	rm -f build/*

.PHONY: all run clean # Это команды, а не файлы