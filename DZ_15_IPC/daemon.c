// #define _XOPEN_SOURCE 700 // Позволяет использовать POSIX-функции (sigaction и др.)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>
#include <signal.h>
#include <ctype.h>

#define CONFIG_FILE "daemon.conf"
#define MAX_PATH 256

// Глобальные настройки
char file_to_watch[MAX_PATH] = "/tmp/target_file.txt";
char current_socket_path[MAX_PATH] = "/tmp/file_size_daemon.sock";
volatile sig_atomic_t reload_requested = 0;

void trim(char *s) {
    char *p = s;
    int l = strlen(p);
    while(l > 0 && isspace(p[l - 1])) p[--l] = 0;
    while(*p && isspace(*p)) { p++; l--; }
    memmove(s, p, l + 1);
}

void load_config(char *f_path, char *s_path) {
    FILE *fp = fopen(CONFIG_FILE, "r");
    if (!fp) return;
    char line[512];
    while (fgets(line, sizeof(line), fp)) {
        if (line[0] == '#' || line[0] == '\n') continue;
        char *key = strtok(line, "=");
        char *value = strtok(NULL, "=");
        if (key && value) {
            trim(key); trim(value);
            if (strcmp(key, "FILE_PATH") == 0) strncpy(f_path, value, MAX_PATH - 1);
            else if (strcmp(key, "SOCKET_PATH") == 0) strncpy(s_path, value, MAX_PATH - 1);
        }
    }
    fclose(fp);
}

int create_socket(const char *path) {
    int fd;
    struct sockaddr_un addr;
    if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) return -1;

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, path, sizeof(addr.sun_path) - 1);

    unlink(path); // Удаляем старый файл сокета, если он остался
    if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        close(fd);
        return -1;
    }
    if (listen(fd, 5) == -1) {
        close(fd);
        return -1;
    }
    return fd;
}

void handle_sighup(int sig) {
    (void)sig;
    reload_requested = 1;
}

void daemonize() {
    pid_t pid = fork();
    if (pid < 0) exit(1);
    if (pid > 0) exit(0);
    setsid();
    signal(SIGCHLD, SIG_IGN);
    pid = fork();
    if (pid < 0) exit(1);
    if (pid > 0) exit(0);
    umask(0);
    chdir("/");
    for (int x = sysconf(_SC_OPEN_MAX); x >= 0; x--) close(x);
}

int main(int argc, char *argv[]) {
    load_config(file_to_watch, current_socket_path);

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handle_sighup;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0; // Важно: убираем SA_RESTART, чтобы прервать accept() при сигнале

    if (sigaction(SIGHUP, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }

    if (argc > 1 && strcmp(argv[1], "-d") == 0) daemonize();

    int server_fd = create_socket(current_socket_path);
    if (server_fd == -1) exit(1);

    while (1) {
        int client_fd = accept(server_fd, NULL, NULL);

        if (client_fd < 0) {
            if (errno == EINTR && reload_requested) {
                char new_f_path[MAX_PATH], new_s_path[MAX_PATH];
                strcpy(new_f_path, file_to_watch);
                strcpy(new_s_path, current_socket_path);

                load_config(new_f_path, new_s_path);
                reload_requested = 0;

                // Если путь к сокету изменился — пересоздаем его
                if (strcmp(new_s_path, current_socket_path) != 0) {
                    int new_fd = create_socket(new_s_path);
                    if (new_fd != -1) {
                        close(server_fd);
                        unlink(current_socket_path);
                        server_fd = new_fd;
                        strcpy(current_socket_path, new_s_path);
                    }
                }
                strcpy(file_to_watch, new_f_path);
                continue;
            }
            continue;
        }

        struct stat st;
        char resp[512];
        if (stat(file_to_watch, &st) == 0) 
            snprintf(resp, sizeof(resp), "File: %s, Size: %ld\n", file_to_watch, st.st_size);
        else 
            snprintf(resp, sizeof(resp), "Error accessing %s: %s\n", file_to_watch, strerror(errno));

        send(client_fd, resp, strlen(resp), 0);
        close(client_fd);
    }
    return 0;
}
