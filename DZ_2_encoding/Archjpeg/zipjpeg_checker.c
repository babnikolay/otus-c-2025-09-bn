/*
Программу нужно скомпилировать с флагами
Для компиляции программы с флагами, указанными в задании, используйте следующую команду в терминале:
gcc -Wall -Wextra -Wpedantic -std=c11 -o rarjpeg_checker rarjpeg_checker.c

Объяснение работы программы:
1. Прием аргументов командной строки: Программа ожидает, что файл будет передан в качестве аргумента командной строки.
2. Проверка файла: Открывается файл, и его содержимое анализируется. Если в конце файла обнаруживается "Rar!", значит, это Rarjpeg.
3. Определение ZIP-архива: Далее программа пробует прочитать заголовок файла. Если он соответствует заголовку ZIP, программа выводит сообщение о том, что файл является zip-архивом.
4. Чтение содержимого файло: Программа в текущем виде не разбирает содержимое ZIP-архива, но оставляет комментарии, что это можно сделать.
Этот код можно использовать в рамках вашей задачи, добавив более детальную логику для разбора ZIP-архивов в случае необходимости.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define EOCD_SIGNATURE 0x06054b50
#define LFH_SIGNATURE 0x04034b50
#define CDFH_SIGNATURE 0x02014b50

#pragma pack(1)
struct EOCD {
    uint32_t signature;                     // Обязательная сигнатура, равна 0x06054b50
    uint16_t diskNumber;                    // Номер диска
    uint16_t startDiskNumber;               // Номер диска, где находится начало Central Directory
    uint16_t numberCentralDirectoryRecord;  // Количество записей в Central Directory в текущем диске
    uint16_t totalCentralDirectoryRecord;   // Всего записей в Central Directory
    uint32_t sizeOfCentralDirectory;        // Размер Central Directory
    uint32_t centralDirectoryOffset;        // Смещение Central Directory
    uint16_t commentLength;                 // Длина комментария
    uint8_t *comment;                       // Комментарий (длиной commentLength)
};
#pragma pack()

#pragma pack(1)
struct LocalFileHeader {
    uint32_t signature;             // Сигнатура, равная 0x04034b50
    uint16_t versionNeeded;         // Версия, необходимая для извлечения
    uint16_t generalPurposeBit;     // Общие флаги
    uint16_t compressionMethod;     // Метод сжатия
    uint16_t lastModifiedTime;      // Время последнего изменения файла
    uint16_t lastModifiedDate;      // Дата последнего изменения файла
    uint32_t crc32;                 // CRC-32
    uint32_t compressedSize;        // Размер сжатого файла
    uint32_t uncompressedSize;      // Размер распакованного файла
    uint16_t fileNameLength;        // Длина имени файла
    uint16_t extraFieldLength;      // Длина дополнительного поля
    // uint8_t *filename;              // Название файла (размером filenameLength)
    // uint8_t *extraField;            // Дополнительные данные (размером extraFieldLength)
};
#pragma pack()

#pragma pack(1)
struct CentralDirectoryFileHeader {
    uint32_t signature;                 // Обязательная сигнатура, равна 0x02014b50 
    uint16_t versionMadeBy;             // Версия для создания
    uint16_t versionToExtract;          // Минимальная версия для распаковки
    uint16_t generalPurposeBitFlag;     // Битовый флаг
    uint16_t compressionMethod;         // Метод сжатия (0 - без сжатия, 8 - deflate)
    uint16_t modificationTime;          // Время модификации файла
    uint16_t modificationDate;          // Дата модификации файла
    uint32_t crc32;                     // Контрольная сумма
    uint32_t compressedSize;            // Сжатый размер
    uint32_t uncompressedSize;          // Несжатый размер
    uint16_t filenameLength;            // Длина название файла
    uint16_t extraFieldLength;          // Длина поля с дополнительными данными
    uint16_t fileCommentLength;         // Длина комментариев к файлу
    uint16_t diskNumber;                // Номер диска
    uint16_t internalFileAttributes;    // Внутренние аттрибуты файла
    uint32_t externalFileAttributes;    // Внешние аттрибуты файла
    uint32_t localFileHeaderOffset;     // Смещение до структуры LocalFileHeader
    uint8_t *filename;                  // Имя файла (длиной filenameLength)
    uint8_t *extraField;                // Дополнительные данные (длиной extraFieldLength)
    uint8_t *fileComment;               // Комментарий к файла (длиной fileCommentLength)
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

    // Смещение на структуру с конца файла
    fseek(file, 0, SEEK_END);
    size_t file_size = ftell(file);
    printf("Длина файла: %ld\n", file_size);

    struct EOCD eocd;
    eocd.signature = 0;
    printf("sizeof(eocd): %ld\n", sizeof(eocd));
    // Перемещение к началу поиска
    for (long i = file_size - 4; i >= 0; i--) {
        fseek(file, i, SEEK_SET);
        fread(&eocd.signature, sizeof(eocd.signature), 1, file);

        if (eocd.signature == EOCD_SIGNATURE) {
            printf("EOCD сигнатура найдена на смещении: %ld\n", i);
            printf("eocd.signature: %#.8x\n", eocd.signature);
            printf("\nЭто ZIP файл\n\n");

            fseek(file, i, SEEK_SET);
            fread(&eocd, sizeof(eocd), 1, file);
            printf("EOCD Details:\n");
            printf("Сигнатура: %#.8x\n", eocd.signature);
            printf("Номер диска, где находится начало Central Directory: %u\n", eocd.diskNumber);
            printf("Начало номера диска, где находится начало Central Directory: %u\n", eocd.startDiskNumber);
            printf("Количество записей в Central Directory в текущем диске: %u\n", eocd.numberCentralDirectoryRecord);
            printf("Всего записей в Central Directory: %u\n", eocd.totalCentralDirectoryRecord);
            printf("Размер Central Directory: %u bytes\n", eocd.sizeOfCentralDirectory);
            printf("Смещение Central Directory: %u bytes\n", eocd.centralDirectoryOffset);
            printf("Длина комментария: %u\n", eocd.commentLength);
            printf("\n");
            break;
        }
    }

    if (eocd.signature != EOCD_SIGNATURE) {
        printf("eocd.signature: %#.8x\n", eocd.signature);
        printf("\nЭто не ZIP файл\n\n");
        fclose(file);
        return 1;
    }

    struct CentralDirectoryFileHeader cdfh;
    cdfh.signature = 0;
    printf("sizeof(cdfh): %ld\n\n", sizeof(cdfh));

    for (long i = file_size - sizeof(eocd); i >= 0; i--) {
        fseek(file, i, SEEK_SET);
        fread(&cdfh.signature, sizeof(cdfh.signature), 1, file);

        if (cdfh.signature == CDFH_SIGNATURE) {
            printf("CDFH сигнатура найдена на смещении: %ld\n", i);
            printf("cdfh.signature: %#.8x\n", cdfh.signature);

            fseek(file, i, SEEK_SET);
            fread(&cdfh, sizeof(cdfh), 1, file);
            printf("CDFH Details:\n");
            printf("Сигнатура: %#.8x\n", cdfh.signature);
            printf("Смещение до структуры LocalFileHeader: %u\n", cdfh.localFileHeaderOffset);
            break;
        }
    }

    fseek(file, 0, SEEK_SET);
    size_t file_pos = ftell(file);
    printf("\nПоиск заголовков lfh с позиции в файле: %ld\n", file_pos);

    fseek(file, cdfh.localFileHeaderOffset, SEEK_SET);
    struct LocalFileHeader lfh;
    lfh.signature = 0;
    printf("\nsizeof(lfh): %ld\n\n", sizeof(lfh));
    unsigned int j = 0;
    for (long unsigned int i = 0; i <= (file_size - sizeof(cdfh)); i++) {
        fseek(file, i, SEEK_SET);
        fread(&lfh.signature, sizeof(lfh.signature), 1, file);
        if (lfh.signature == LFH_SIGNATURE) {
            printf("\n");
            j++;
            printf("j = %d\n", j);
            printf("LFH сигнатура найдена на смещении: %ld\n", i);
            printf("lfh.signature: %#.8x\n\n", lfh.signature);

            size_t file_pos = ftell(file);
            printf("file_pos: %ld\n\n", file_pos);
            fseek(file, i, SEEK_SET);
            fread(&lfh, sizeof(lfh), 1, file);
            
            printf("LFH Details:\n");
            printf("Сигнатура: %#.8x\n", lfh.signature);
            printf("Версия, необходимая для извлечения: %u\n", lfh.versionNeeded);
            printf("Общие флаги: %u\n", lfh.generalPurposeBit);
            printf("Метод сжатия: %u\n", lfh.compressionMethod);
            printf("Время последнего изменения файла: %u\n", lfh.lastModifiedTime);
            printf("Дата последнего изменения файла: %u\n", lfh.lastModifiedDate);
            printf("CRC-32: %u\n", lfh.crc32);
            printf("Размер сжатого файла: %u\n", lfh.compressedSize);
            printf("Размер распакованного файла: %u\n", lfh.uncompressedSize);
            printf("Длина имени файла: %u\n", lfh.fileNameLength);
            printf("Длина дополнительного поля: %u\n", lfh.extraFieldLength);
            printf("\n");
            
            unsigned char *filename = (uint8_t *)malloc(lfh.fileNameLength + 1);
            if (filename == NULL) {
                fprintf(stderr, "Ошибка выделения памяти\n");
            }
            else {
                fread(filename, 1, lfh.fileNameLength, file);
                filename[lfh.fileNameLength] = '\0';
                printf("Имя файла: %s\n", filename);

                if (filename) {
                    free(filename);
                }
            }
            printf("\n");
        }
    }

    printf("Количество файлов в архиве: %u\n\n", j);

    fclose(file);
 
    return 0;
}
