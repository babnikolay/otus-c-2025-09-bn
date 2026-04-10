#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <poll.h>

#define BUFFER_SIZE 8192
#define TIMEOUT_MS 5000 

void clean_print(unsigned char *buf, ssize_t len) {
    for (ssize_t i = 0; i < len; i++) {
        if (buf[i] == 255) { // Пропуск IAC
            if (i + 2 < len) i += 2;
            else if (i + 1 < len) i += 1;
            continue;
        }
        if (buf[i] != '\r') putchar(buf[i]);
    }
    fflush(stdout);
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Использование: %s <шрифт> <текст...>\n", argv[0]);
        return 1;
    }

    // 1. Склеиваем текст
    char text[BUFFER_SIZE] = "";
    for (int i = 2; i < argc; i++) {
        strncat(text, argv[i], sizeof(text) - strlen(text) - 1);
        if (i < argc - 1) strncat(text, " ", sizeof(text) - strlen(text) - 1);
    }

    struct hostent *server = gethostbyname("telehack.com");
    if (!server) return 1;

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr = { .sin_family = AF_INET, .sin_port = htons(23) };
    memcpy(&addr.sin_addr.s_addr, server->h_addr_list[0], server->h_length);

    if (connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("Connect failed");
        return 1;
    }

    struct pollfd pfd = { .fd = sockfd, .events = POLLIN };
    unsigned char buffer[BUFFER_SIZE];

    // 2. Ждем ПЕРВОГО приглашения (точка), чтобы понять, что сервер готов
    while (poll(&pfd, 1, 3000) > 0) {
        ssize_t n = recv(sockfd, buffer, sizeof(buffer) - 1, 0);
        if (n <= 0) break;
        if (memchr(buffer, '.', n)) break;
    }

    // 3. Отправляем команду
    char cmd[BUFFER_SIZE + 512];
    snprintf(cmd, sizeof(cmd), "figlet -f %s %s\r\n", argv[1], text);
    send(sockfd, cmd, strlen(cmd), 0);

    // 4. Читаем всё подряд. Первое, что придет — это эхо команды.
    int lines_to_skip = 1; 
    int got_art = 0; // Флаг наличия полезного вывода

    while (poll(&pfd, 1, TIMEOUT_MS) > 0) {
        ssize_t n = recv(sockfd, buffer, sizeof(buffer) - 1, 0);
        if (n <= 0) break;
        buffer[n] = '\0';

        unsigned char *ptr = buffer;
        
        // Пропуск строки-эха
        while (lines_to_skip > 0 && n > 0) {
            unsigned char *newline = memchr(ptr, '\n', n);
            if (newline) {
                n -= (newline - ptr + 1);
                ptr = newline + 1;
                lines_to_skip--;
            } else {
                n = 0; 
            }
        }

        if (n <= 0) continue;

        // Ищем маркер конца
        char *end = strstr((char*)ptr, "\r\n.");
        if (end) {
            size_t final_len = (unsigned char*)end - ptr;
            if (final_len > 0) {
                clean_print(ptr, final_len);
                got_art = 1;
            }
            break;
        }
        
        // Если мы здесь и печатаем что-то, кроме пустоты
        if (n > 0) {
            clean_print(ptr, n);
            got_art = 1;
        }
    }

    if (!got_art) {
        fprintf(stderr, "\nОшибка: Сервер вернул пустой ответ. Проверьте название шрифта '%s'.\n", argv[1]);
    } else {
        printf("\n");
    }

    close(sockfd);
    return 0;

}
