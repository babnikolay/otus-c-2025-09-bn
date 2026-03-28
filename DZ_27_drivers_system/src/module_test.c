#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <time.h>

#define DEVICE "/dev/bn_device"
#define FIFO_SIZE 1024

// Функция для логирования с меткой времени
#define log_msg(tag, fmt, ...) do { \
    time_t now = time(NULL); \
    struct tm *t = localtime(&now); \
    printf("[%02d:%02d:%02d] [%s] " fmt "\n", \
           t->tm_hour, t->tm_min, t->tm_sec, tag, ##__VA_ARGS__); \
} while (0)

void* overflow_writer() {
    int fd = open(DEVICE, O_WRONLY);
    if (fd < 0) { perror("Writer: Open failed"); return NULL; }

    char big_data[2*FIFO_SIZE];
    memset(big_data, 'A', 2*FIFO_SIZE);

    log_msg("Writer", "Attempting to write 2*%d bytes (FIFO is only %d)...", FIFO_SIZE, FIFO_SIZE);
    
    // Этот вызов должен заблокироваться после записи первых 32 байт
    ssize_t written = write(fd, big_data, 2*FIFO_SIZE);
    
    if (written > 0) {
        log_msg("Writer", "Finished! Successfully wrote %zd bytes total.", written);
    } else {
        perror("Writer: Write failed");
    }

    close(fd);
    return NULL;
}

void* slow_reader() {
    // Ждем 3 секунды, пока писатель гарантированно упрется в потолок
    sleep(3);
    
    int fd = open(DEVICE, O_RDONLY);
    if (fd < 0) { perror("Reader: Open failed"); return NULL; }

    char rx_buf[FIFO_SIZE];
    log_msg("Reader", "Waking up to read %d bytes and free some space...", FIFO_SIZE);
    
    ssize_t bytes = read(fd, rx_buf, FIFO_SIZE);
    if (bytes > 0) {
        log_msg("Reader", "Read %d bytes. Space should be free now.", FIFO_SIZE);
    }

    close(fd);
    return NULL;
}

int main() {
    pthread_t r, w;

    printf("--- Starting Overflow Test (FIFO Size: %d) ---\n", FIFO_SIZE);

    pthread_create(&w, NULL, overflow_writer, NULL);
    pthread_create(&r, NULL, slow_reader, NULL);

    pthread_join(w, NULL);
    pthread_join(r, NULL);

    printf("--- Test Finished ---\n");
    return 0;
}
