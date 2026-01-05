#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#pragma pack(1)
struct LocalFileHeader {
    uint32_t signature;            // Сигнатура, равная 0x04034b50
    uint16_t versionNeeded;        // Версия, необходимая для извлечения
    uint16_t generalPurposeBit;    // Общие флаги
    uint16_t compressionMethod;     // Метод сжатия
    uint16_t lastModifiedTime;      // Время последнего изменения файла
    uint16_t lastModifiedDate;      // Дата последнего изменения файла
    uint32_t crc32;                 // CRC-32
    uint32_t compressedSize;       // Размер сжатого файла
    uint32_t uncompressedSize;     // Размер распакованного файла
    uint16_t fileNameLength;       // Длина имени файла
    uint16_t extraFieldLength;     // Длина дополнительного поля
};

#pragma pack()

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Используйте: %s <имя_файла>\n", argv[0]);
        return 1;
    }

    const char *filename = argv[1];
    FILE *file = fopen(filename, "rb");
    if (!file) {
        perror("Не удалось открыть файл");
        return 1;
    }

    // Читаем локальные записи
    while (1) {
        struct LocalFileHeader header;
        fread(&header, sizeof(struct LocalFileHeader), 1, file);

        // Проверяем сигнатуру
        if (header.signature != 0x04034b50) {
            printf("\nСигнатура не соответствует\n\n");
            break;  // Если сигнатура не соответствует, выходим из цикла
        }

        // Читаем имя файла
        char *fileName = (char *)malloc(header.fileNameLength + 1);
        fread(fileName, header.fileNameLength, 1, file);
        fileName[header.fileNameLength] = '\0';  // Нулевой терминатор

        // Пропускаем дополнительное поле
        fseek(file, header.extraFieldLength, SEEK_CUR);

        // Извлечение данных
        uint8_t *fileData = (uint8_t *)malloc(header.compressedSize);
        fread(fileData, header.compressedSize, 1, file);

        // Здесь можно распаковать сжатые данные, если это необходимо

        // Выводим имя файла
        printf("Extracted: %s\n", fileName);

        // Освобождаем память
        free(fileName);
        free(fileData);
    }

    fclose(file);
    return 0;
}
