/*
Эта программа на C, которая использует несколько системных вызовов: open, write, read, info, close и unlink.
Она открывает файл, записывает строку в файл, считывает её, получает информацию о файле, 
затем закрывает его и удаляет.
*/
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

int main() {
    const char *filename = "/tmp/test_ftrace.txt";
    const char *text = "Hello, ftrace!\n";
    char buffer[50];
    int fd;

    printf("\n === FTRACE SYSCALL DEMO ===\n\n");
    printf("Ожидаем нажатия Enter: ");
    fflush(stdout);
    getchar();  // Wait for user to set up ftrace

    printf("\n=== SYSCALL OPERATIONS START ===\n");

    // Системный вызов open
    printf("1. Открытие файла (open syscall)...\n");
    fd = open(filename, O_CREAT | O_RDWR | O_TRUNC, 0644);
    if (fd < 0) {
        perror("Ошибка при открытии файла");
        exit(EXIT_FAILURE); 
    }
    printf("   File opened, fd = %d\n", fd);

    // Системный вызов write
    printf("2. Запись в файл (write syscall)...\n");
    ssize_t bytes_written = write(fd, text, strlen(text));
    if (bytes_written < 0) {
        perror("Ошибка при записи в файл");
        close(fd);
        exit(EXIT_FAILURE);
    }
    printf("   Записано в файл %ld байт\n", bytes_written);

    // Позиционирование указателя файла в начало
    lseek(fd, 0, SEEK_SET);

    // Системный вызов read
    printf("3. Чтение из файла (read syscall)...\n");
    ssize_t bytesRead = read(fd, buffer, sizeof(buffer) - 1);
    if (bytesRead == -1) {
        perror("Ошибка при чтении из файла");
        close(fd);
        exit(EXIT_FAILURE);
    }

    buffer[bytesRead] = '\0';  // Завершаем строку

    printf("   Содержимое файла: %s", buffer);

    printf("4. Получение информации о файле (newfstat syscall)...\n");
    struct stat file_stat;
    if (fstat(fd, &file_stat) == 0) {
        printf("   Размер файла: %ld байт\n", file_stat.st_size);
        printf("   Доступы к файлу: %o\n", file_stat.st_mode & 0777);
    }

    // Системный вызов close
    printf("5. Закрытие файла (close syscall)...\n");
    if (close(fd) < 0) {
        perror("Ошибка при закрытии файла");
        exit(EXIT_FAILURE);
    }
    printf("   Файл закрыт\n");

    // Системный вызов delete
    printf("6. Удаление файла (unlink syscall)...\n");
    if (unlink("/tmp/test_ftrace.txt") == 0) {
        printf("   Файл удалён\n");
    } else {
        perror("Ошибка unlink. Файл не удален");
    }

    printf("\n=== SYSCALL OPERATIONS END ===\n");
    printf("Проверьте /tmp/ftrace_output.txt для трассировки системных вызовов!\n");

    return 0;
}
