#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <poll.h>
#include <stdbool.h>

#define BUFFER_SIZE 8192
#define TIMEOUT_MS 5000

// Гарантированная отправка всех байт
ssize_t send_all(int fd, const char *buf, size_t len) {
    size_t total_sent = 0;
    while (total_sent < len) {
        ssize_t n = send(fd, buf + total_sent, len - total_sent, 0);
        if (n <= 0) return -1; 
        total_sent += n;
    }
    return (ssize_t)total_sent;
}

// Очистка вывода от мусора Telnet (IAC) и лишних возвратов каретки
bool clean_print(unsigned char *buf, ssize_t len) {
    bool printed = false;
    for (ssize_t i = 0; i < len; i++) {
        if (buf[i] == 255) { // Telnet Interpret As Command
            if (i + 2 < len) i += 2;
            else if (i + 1 < len) i += 1;
            continue;
        }
        if (buf[i] != '\r') {
            putchar(buf[i]);
            printed = true;
        }
    }
    fflush(stdout);
    return printed;
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Использование: %s <шрифт> <текст...>\n", argv[0]);
        return 1;
    }

    // 1. Склеиваем текст (безопасно через snprintf)
    char text[BUFFER_SIZE] = "";
    size_t offset = 0;
    for (int i = 2; i < argc; i++) {
        int written = snprintf(text + offset, sizeof(text) - offset, "%s%s", 
                               argv[i], (i < argc - 1) ? " " : "");
        if (written < 0 || (size_t)written >= sizeof(text) - offset) break;
        offset += written;
    }

    struct hostent *server = gethostbyname("telehack.com");
    if (!server) {
        fprintf(stderr, "Ошибка DNS\n");
        return 1;
    }

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Ошибка сокета");
        return 1;
    }

    struct sockaddr_in addr = { .sin_family = AF_INET, .sin_port = htons(23) };
    memcpy(&addr.sin_addr.s_addr, server->h_addr_list[0], server->h_length);

    if (connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("Ошибка подключения");
        close(sockfd);
        return 1;
    }

    struct pollfd pfd = { .fd = sockfd, .events = POLLIN };
    unsigned char buffer[BUFFER_SIZE];

    // 2. Ждем готовности сервера (символ '.')
    while (poll(&pfd, 1, 3000) > 0) {
        ssize_t n = recv(sockfd, buffer, sizeof(buffer) - 1, 0);
        if (n <= 0) break;
        if (memchr(buffer, '.', n)) break;
    }

    // 3. Формируем и отправляем команду
    char cmd[BUFFER_SIZE + 128];
    snprintf(cmd, sizeof(cmd), "figlet -f %s %s\r\n", argv[1], text);
    
    if (send_all(sockfd, cmd, strlen(cmd)) < 0) {
        perror("Ошибка отправки команды");
        close(sockfd);
        return 1;
    }

    // 4. Читаем ответ
    int lines_to_skip = 1; // Пропускаем эхо самой команды
    bool got_art = false;

    while (poll(&pfd, 1, TIMEOUT_MS) > 0) {
        ssize_t n = recv(sockfd, buffer, sizeof(buffer) - 1, 0);
        if (n <= 0) break;
        buffer[n] = '\0';

        unsigned char *ptr = buffer;
        ssize_t current_len = n;

        // Пропуск первой строки (эха)
        while (lines_to_skip > 0 && current_len > 0) {
            unsigned char *newline = memchr(ptr, '\n', current_len);
            if (newline) {
                current_len -= (newline - ptr + 1);
                ptr = newline + 1;
                lines_to_skip--;
            } else {
                current_len = 0; 
            }
        }

        if (current_len <= 0) continue;

        // Поиск маркера конца команды в Telehack (точка на новой строке)
        char *end = strstr((char*)ptr, "\r\n.");
        if (end) {
            size_t final_len = (unsigned char*)end - ptr;
            if (final_len > 0) {
                if (clean_print(ptr, final_len)) got_art = true;
            }
            break;
        }

        if (clean_print(ptr, current_len)) got_art = true;
    }

    if (!got_art) {
        fprintf(stderr, "\n[!] Ответ пуст. Проверьте шрифт '%s' или текст.\n", argv[1]);
    } else {
        printf("\n");
    }

    close(sockfd);
    return 0;
}
