int kernel_main(void) {
    int offset_buf = 80 * 2 * 2; // Смещение для буфера, чтобы можно было увидеть прошлые логи
                                 // 80 символов в строке, пропускаем 2 строки, где 2 байта на символ
    volatile char *video_memory = (volatile char *)(0xB8000 + offset_buf);

    video_memory[0] = 'O';
    video_memory[1] = 0x0F;
    video_memory[2] = 'K';
    video_memory[3] = 0x0F;

    // Бесконечный цкил
    while (1) {
        // Вызываем ассемблерную инструкцию hlt
        __asm__ __volatile__("hlt"); 
    }
    return 0;
}
