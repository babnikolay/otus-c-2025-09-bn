#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

#define BUFFER_SIZE 65536 // Буфер 64 КБ
#define BUFFER 256
uint32_t crc32_table[BUFFER];

// Генерируем таблицу программно, чтобы исключить ошибки копирования
void generate_table() {
    uint32_t polynomial = 0xEDB88320;
    for (uint32_t i = 0; i < BUFFER; i++) {
        uint32_t c = i;
        for (int j = 0; j < 8; j++) {
            if (c & 1) c = polynomial ^ (c >> 1);
            else c >>= 1;
        }
        crc32_table[i] = c;
    }
}

uint32_t calculate_crc32(FILE *f) {
    uint32_t crc = 0xFFFFFFFF;
    uint8_t buffer[BUFFER_SIZE];
    size_t bytes_read;

    rewind(f);  // Сбрасываем указатель в начало файла

    while ((bytes_read = fread(buffer, 1, sizeof(buffer), f)) > 0) {
        for (size_t i = 0; i < bytes_read; i++) {
            uint8_t index = (crc ^ buffer[i]) & 0xFF;
            crc = (crc >> 8) ^ crc32_table[index];
        }
    }

    return crc ^ 0xFFFFFFFF;
}

int main(int argc, char *argv[]) {
    // Фиксируем время начала
    time_t start_time = time(NULL);
    printf("Начало: %s\n", ctime(&start_time));

    if (argc != 2) {
        fprintf(stderr, "Использование: %s <путь_к_файлу>\n", argv[0]);
        return 1;
    }

    generate_table(); // Инициализируем таблицу перед расчетом

    FILE *file = fopen(argv[1], "rb");
    if (!file) {
        perror("Ошибка открытия файла");
        return 1;
    }

    uint32_t result = calculate_crc32(file);
    
    if (ferror(file)) {
        fprintf(stderr, "Ошибка при чтении файла.\n");
        fclose(file);
        return 1;
    }

    printf("%08x\n", result);

    fclose(file);

    // 2. Фиксируем время окончания
    time_t end_time = time(NULL);
    printf("\nОкончание: %s\n", ctime(&end_time));

    // 3. Вычисляем разницу (длительность)
    double diff = difftime(end_time, start_time);
    printf("Программа работала %.0f сек.\n", diff);

    return 0;
}
