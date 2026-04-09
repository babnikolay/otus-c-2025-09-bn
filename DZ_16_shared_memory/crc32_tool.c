#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>

// #define BUFFER_SIZE 65536 // Буфер 64 КБ
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

uint32_t calculate_crc32(const uint8_t *data, size_t length) {
    uint32_t crc = 0xFFFFFFFF;

    for (size_t i = 0; i < length; i++) {
        uint8_t index = (crc ^ data[i]) & 0xFF;
        crc = (crc >> 8) ^ crc32_table[index];
    }

    return crc ^ 0xFFFFFFFF;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Использование: %s <путь_к_файлу>\n", argv[0]);
        return 1;
    }

    // Фиксируем время начала
    time_t start_time = time(NULL);
    printf("Начало: %s\n", ctime(&start_time));

    generate_table(); // Инициализируем таблицу перед расчетом

    int fd = open(argv[1], O_RDONLY);
    if (fd == -1) {
        perror("Ошибка открытия файла");
        return 1;
    }

    // Получаем размер файла
    struct stat st;
    if (fstat(fd, &st) == -1) {
        perror("Ошибка fstat");
        close(fd);
        return 1;
    }

    // Отображаем файл в память (Shared Memory)
    // PROT_READ — только чтение, MAP_SHARED — изменения видны другим процессам
    uint8_t *mapped = mmap(NULL, st.st_size, PROT_READ, MAP_SHARED, fd, 0);
    if (mapped == MAP_FAILED) {
        perror("Ошибка mmap");
        close(fd);
        return 1;
    }
    
    uint32_t result = calculate_crc32(mapped, st.st_size);
    
    printf("%08x\n", result);

    // Освобождаем ресурсы
    munmap(mapped, st.st_size);
    close(fd);

    // 2. Фиксируем время окончания
    time_t end_time = time(NULL);
    printf("\nОкончание: %s\n", ctime(&end_time));

    // 3. Вычисляем разницу (длительность)
    double diff = difftime(end_time, start_time);
    printf("Программа работала %.0f сек.\n", diff);

    return 0;
}
