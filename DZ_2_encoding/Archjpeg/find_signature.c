#include <stdio.h>
#include <stdint.h>

#define EOCD_SIG 0x06054b50
#define EOCD_MAX_COMMENT 0xFFFF  // Максимальный размер комментария в EOCD (65535)
#define EOCD_FIXED_SIZE 22       // Размер EOCD без комментария

// Функция ищет EOCD в файле и возвращает смещение EOCD или -1 при ошибке
long find_eocd(FILE *f) {
    if (fseek(f, 0, SEEK_END) != 0) {
        return -1;
    }
    long file_size = ftell(f);
    if (file_size < EOCD_FIXED_SIZE) {
        return -1; // Файл слишком маленький, EOCD не может быть
    }

    // Максимально далеко назад от конца файла ищем EOCD
    long max_search_start = file_size - EOCD_FIXED_SIZE;
    long min_search_start = file_size - EOCD_FIXED_SIZE - EOCD_MAX_COMMENT;
    if (min_search_start < 0) {
        min_search_start = 0;
    }

    uint8_t buffer[4];
    for (long pos = max_search_start; pos >= min_search_start; pos--) {
        if (fseek(f, pos, SEEK_SET) != 0) {
            return -1;
        }
        if (fread(buffer, 1, 4, f) != 4) {
            return -1;
        }
        // Считываем 4 байта как little-endian uint32_t
        uint32_t sig = (uint32_t)buffer[0] | ((uint32_t)buffer[1] << 8) |
                       ((uint32_t)buffer[2] << 16) | ((uint32_t)buffer[3] << 24);
        if (sig == EOCD_SIG) {
            return pos; // Найдена сигнатура EOCD
        }
    }
    return -1; // Не найдена
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <zipfile>\n", argv[0]);
        return 1;
    }

    FILE *f = fopen(argv[1], "rb");
    if (!f) {
        perror("Error opening file");
        return 1;
    }

    long eocd_pos = find_eocd(f);
    if (eocd_pos < 0) {
        printf("EOCD signature not found\n");
    } else {
        printf("EOCD signature found at offset: %ld\n", eocd_pos);
    }

    fclose(f);
    return 0;
}