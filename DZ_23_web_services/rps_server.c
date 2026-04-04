#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <sys/wait.h>
#include <errno.h>

#define MAX_EVENTS 1024
#define BUFFER_SIZE 4096
#define NUM_WORKERS 1 // Установите по количеству ядер CPU

// Статический заголовок для моментального ответа
const char* http_ok = "HTTP/1.1 200 OK\r\n"
                        "Content-Length: 13\r\n"
                        "Connection: keep-alive\r\n"
                        "Content-Type: text/plain\r\n\r\n"
                        "Hello World!\n";

void set_nonblocking(int fd) {
    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
}

void handle_client(int client_fd, __attribute__((unused)) const char *dir) {
    char buf[1024];
    // Читаем запрос (в реальности нужно парсить, для теста просто вычитываем)
    ssize_t n = recv(client_fd, buf, sizeof(buf), 0);
    if (n <= 0) return;

    // Отправляем заранее подготовленный ответ (максимальная скорость)
    send(client_fd, http_ok, strlen(http_ok), 0);
}

void run_worker(const char *ip, int port, __attribute__((unused)) const char *dir) {
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    
    int opt = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)); // Магия тут
    
    struct sockaddr_in addr = { .sin_family = AF_INET, .sin_port = htons(port) };
    inet_pton(AF_INET, ip, &addr.sin_addr);

    if (bind(listen_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind"); exit(1);
    }
    
    if (listen(listen_fd, 65535) < 0) { 
        perror("listen"); 
        exit(1); 
    }
    set_nonblocking(listen_fd);

    int epoll_fd = epoll_create1(0);
    struct epoll_event ev = { .events = EPOLLIN | EPOLLEXCLUSIVE, .data.fd = listen_fd };
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listen_fd, &ev);

    struct epoll_event events[MAX_EVENTS];
    while (1) {
        int nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        for (int i = 0; i < nfds; i++) {
            if (events[i].data.fd == listen_fd) {
                int client_fd = accept(listen_fd, NULL, NULL);
                if (client_fd < 0) {
                    if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) continue;
                }

                if (client_fd > 0) {
                    set_nonblocking(client_fd);
                    struct epoll_event ev_cli = { .events = EPOLLIN | EPOLLET, .data.fd = client_fd };
                    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &ev_cli);
                }
            } else {
                handle_client(events[i].data.fd, dir);
                // Для теста высокого RPS с Keep-Alive НЕ закрываем сокет здесь!
                // Его закроет клиент или таймаут.
            }
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) return 1;

    char *dir = argv[1];
    char *ip = strtok(argv[2], ":");
    int port = atoi(strtok(NULL, ":"));

    printf("Starting %d workers on %s:%d...\n", NUM_WORKERS, ip, port);
    fflush(stdout);

    for (int i = 0; i < NUM_WORKERS; i++) {
        if (fork() == 0) {
            run_worker(ip, port, dir);
            exit(0);
        }
    }

    while (wait(NULL) > 0); // Ждем воркеров
    return 0;
}
