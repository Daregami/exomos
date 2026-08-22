import struct
import sys
import os

SECTOR = 512
EXOMFS_START = 2048  # сектор начала ФС
MAGIC = 0x45584F4D   # "EXOM"

def main():
    if len(sys.argv) < 3:
        print("Использование: mkexomfs.py <disk_image> <dir>")
        return

    image_path = sys.argv[1]
    dir_path = sys.argv[2]

    # Собираем все файлы из папки
    files = sorted([
        os.path.join(dir_path, f)
        for f in os.listdir(dir_path)
        if os.path.isfile(os.path.join(dir_path, f))
    ])

    if not files:
        print("Нет файлов в", dir_path)
        return

    # Заголовок: magic + file_count
    header = struct.pack('<II', MAGIC, len(files))

    # Записи файлов
    # Данные начинаются через 1 сектор после таблицы
    data_offset = 1  # в секторах от начала ФС
    entries = b''
    file_data = b''

    for f in files:
        with open(f, 'rb') as fh:
            data = fh.read()

        name = os.path.basename(f)
        name_bytes = name.encode('ascii')[:24].ljust(24, b'\x00')

        entry = struct.pack('<24sII', name_bytes, data_offset, len(data))
        entries += entry

        # Выравниваем данные по секторам
        padded = data + b'\x00' * (SECTOR - len(data) % SECTOR) if len(data) % SECTOR else data
        file_data += padded
        data_offset += len(padded) // SECTOR

    # Записываем в образ
    with open(image_path, 'r+b') as img:
        img.seek(EXOMFS_START * SECTOR)
        img.write(header + entries)
        # Дополняем первый сектор нулями
        written = len(header) + len(entries)
        img.write(b'\x00' * (SECTOR - written))
        # Записываем данные файлов
        img.write(file_data)

    print(f"exomfs: {len(files)} files packed")

if __name__ == '__main__':
    main()