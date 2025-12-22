/*
Комментарии
Использование аргументов командной строки: Программа принимает путь к входному файлу, кодировку и путь к выходному файлу.
Обработка ошибок: При открытии файлов производится проверка на наличие ошибок. Если файл не может быть открыт, программа выводит ошибку и завершает работу.
Конвертация каждого из форматов: Каждая функция конвертации обрабатывает соответствующий формат, конверт
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 1024

// Прототипы функций
void convert_cp1251_to_utf8(FILE *input, FILE *output);
void convert_koi8r_to_utf8(FILE *input, FILE *output);
void convert_iso8859_5_to_utf8(FILE *input, FILE *output);
int convert_character(int ch, int encoding);

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <input file> <encoding> <output file>\n", argv[0]);
        printf("Supported encodings: CP1251, KOI8-R, ISO-8859-5\n");
        return EXIT_FAILURE;
    }

    FILE *input = fopen(argv[1], "rb");
    if (!input) {
        perror("Error opening input file");
        return EXIT_FAILURE;
    }

    FILE *output = fopen(argv[3], "wb");
    if (!output) {
        perror("Error opening output file");
        fclose(input);
        return EXIT_FAILURE;
    }

    if (strcmp(argv[2], "CP1251") == 0) {
        convert_cp1251_to_utf8(input, output);
    } else if (strcmp(argv[2], "KOI8-R") == 0) {
        convert_koi8r_to_utf8(input, output);
    } else if (strcmp(argv[2], "ISO-8859-5") == 0) {
        convert_iso8859_5_to_utf8(input, output);
    } else {
        fprintf(stderr, "Unsupported encoding: %s\n", argv[2]);
        fclose(input);
        fclose(output);
        return EXIT_FAILURE;
    }

    fclose(input);
    fclose(output);
    return EXIT_SUCCESS;
}

void convert_cp1251_to_utf8(FILE *input, FILE *output) {
    int ch;
    while ((ch = fgetc(input)) != EOF) {
        if (ch < 0x80) {
            fputc(ch, output); // ASCII characters are the same
        } else {
            // Convert CP-1251 to UTF-8
            int unicode = ch - 0x80 + 0x0400; // Simple conversion
            if (unicode < 0x800) {
                fprintf(output, "%c%c", 0xC0 | (unicode >> 6), 0x80 | (unicode & 0x3F));
            } else {
                fprintf(output, "%c%c%c", 0xE0 | (unicode >> 12), 0x80 | ((unicode >> 6) & 0x3F), 0x80 | (unicode & 0x3F));
            }
        }
    }
}

void convert_koi8r_to_utf8(FILE *input, FILE *output) {
    int ch;
    while ((ch = fgetc(input)) != EOF) {
        // Convert KOI8-R to UTF-8
        int unicode;
        if (ch < 0x80) {
            fputc(ch, output); // ASCII characters are the same
        } else {
            unicode = ch - 0x80 + 0x0400; // Simple conversion
            fprintf(output, "%c%c", 0xC0 | (unicode >> 6), 0x80 | (unicode & 0x3F));
        }
    }
}

void convert_iso8859_5_to_utf8(FILE *input, FILE *output) {
    int ch;
    while ((ch = fgetc(input)) != EOF) {
        if (ch < 0x80) {
            fputc(ch, output); // ASCII characters are the same
        } else {
            int unicode = ch - 0x80 + 0x0400; // Simple conversion
            fprintf(output, "%c%c", 0xC0 | (unicode >> 6), 0x80 | (unicode & 0x3F));
        }
    }
}
