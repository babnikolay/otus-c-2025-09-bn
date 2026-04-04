#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <errno.h>
#include <limits.h>
#include <strings.h>
#include <dirent.h>

#define MAX_EVENTS 1024
#define BUFFER_SIZE 8192

struct mime_entry {
    const char *ext;
    const char *type;
};

struct mime_entry mime_types[] = {
    {".html", "text/html; charset=utf-8"},
    {".htm",  "text/html; charset=utf-8"},
    {".css",  "text/css"},
    {".js",   "application/javascript"},
    {".json", "application/json"},
    {".png",  "image/png"},
    {".jpg",  "image/jpeg"},
    {".jpeg", "image/jpeg"},
    {".gif",  "image/gif"},
    {".svg",  "image/svg+xml"},
    {".ico",  "image/x-icon"},
    {".txt",  "text/plain; charset=utf-8"},
    {".pdf",  "application/pdf"},
    {".zip",  "application/zip"},
    {".mp4",  "video/mp4"},
    {".webm", "video/webm"},
    {".ttf",  "font/ttf"},
    {NULL, NULL} // Маркер конца списка
};

const char* get_mime_type(const char *path) {
    const char *ext = strrchr(path, '.');
    if (!ext) return "application/octet-stream";

    for (int i = 0; mime_types[i].ext != NULL; i++) {
        // strcasecmp для игнорирования регистра (.JPG == .jpg)
        if (strcasecmp(ext, mime_types[i].ext) == 0) {
            return mime_types[i].type;
        }
    }
    return "application/octet-stream";
}

// --- Вспомогательные функции ---

long get_max_path(const char *path) {
    errno = 0;
    long res = pathconf(path, _PC_PATH_MAX);
    return (res == -1 && errno == 0) ? 4096 : res;
}

const char* get_icon(const char *name, mode_t mode) {
    if (S_ISDIR(mode)) return "📁";
    const char *ext = strrchr(name, '.');
    if (!ext) return "📄";
    if (strcasecmp(ext, ".pdf") == 0) return "📕";
    if (strcasecmp(ext, ".html") == 0) return "🌐";
    if (strcasecmp(ext, ".jpg") == 0 || strcasecmp(ext, ".png") == 0) return "🖼️";
    return "📄";
}

void send_directory_listing(int client_fd, const char *abs_path, const char *web_path) {
    DIR *d = opendir(abs_path);
    if (!d) return;

    dprintf(client_fd, "HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=utf-8\r\nConnection: close\r\n\r\n");
    dprintf(client_fd, "<html><head><meta charset='utf-8'><title>Index of %s</title></head><body>", web_path);
    dprintf(client_fd, "<h1>Index of %s</h1><hr><ul>", web_path);

    struct dirent *entry;
    while ((entry = readdir(d)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0) continue;
        
        char full_entry_path[PATH_MAX];
        snprintf(full_entry_path, sizeof(full_entry_path), "%s/%s", abs_path, entry->d_name);
        struct stat st;
        stat(full_entry_path, &st);

        const char *icon = get_icon(entry->d_name, st.st_mode);
        dprintf(client_fd, "<li>%s <a href=\"%s%s%s\">%s</a></li>", 
                icon, web_path, (web_path[strlen(web_path)-1] == '/') ? "" : "/", entry->d_name, entry->d_name);
    }
    dprintf(client_fd, "</ul><hr></body></html>");
    closedir(d);
}

void set_nonblocking(int sock) {
    int opts = fcntl(sock, F_GETFL);
    if (opts < 0) {
        perror("fcntl(F_GETFL)");
        return;
    }
    fcntl(sock, F_SETFL, opts | O_NONBLOCK);
}

// Функция для надежной отправки всего объема данных через sendfile
int send_all_sendfile(int client_fd, int file_fd, off_t size) {
    off_t offset = 0;
    while (offset < size) {
        ssize_t sent = sendfile(client_fd, file_fd, &offset, size - offset);
        if (sent == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // В идеале здесь нужно сохранить состояние и ждать EPOLLOUT,
                // но для простоты в рамках данной структуры подождем немного.
                usleep(1000); 
                continue;
            }
            return -1;
        }
    }
    return 0;
}

// --- Обработка запроса ---

void handle_request(int client_fd, const char *work_dir) {
    char buffer[BUFFER_SIZE];
    ssize_t n = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
    if (n <= 0) return;
    buffer[n] = '\0';

    char method[16], raw_path[PATH_MAX];
    if (sscanf(buffer, "%s %s", method, raw_path) < 2) return;

    char abs_work_dir[PATH_MAX];
    if (!realpath(work_dir, abs_work_dir)) return;

    char full_req_path[PATH_MAX * 2];
    snprintf(full_req_path, sizeof(full_req_path), "%s%s", abs_work_dir, raw_path);
    
    char resolved_path[PATH_MAX];
    char *res = realpath(full_req_path, resolved_path);

    // Защита от Directory Traversal
    if (res == NULL || strncmp(abs_work_dir, resolved_path, strlen(abs_work_dir)) != 0) {
        dprintf(client_fd, 
                "HTTP/1.1 403 Forbidden\r\n"
                "Content-Length: 9\r\n\r\n"
                "Forbidden");
        return;
    }

    struct stat st;
    if (stat(resolved_path, &st) == -1) {
        dprintf(client_fd, 
                "HTTP/1.1 404 Not Found\r\n"
                "Content-Length: 9\r\n\r\n"
                "Not Found");
        return;
    }

    if (S_ISDIR(st.st_mode)) {
        // Проверяем index.html внутри папки
        char index_path[PATH_MAX + 16];
        snprintf(index_path, sizeof(index_path), "%s/index.html", resolved_path);
        struct stat st_idx;
        if (stat(index_path, &st_idx) == 0) {
            int fd = open(index_path, O_RDONLY);
            dprintf(client_fd, 
                    "HTTP/1.1 200 OK\r\n"
                    "Content-Type: text/html\r\n"
                    "Content-Length: %ld\r\n\r\n", 
                    st_idx.st_size);
            sendfile(client_fd, fd, NULL, st_idx.st_size);
            close(fd);
        } else {
            send_directory_listing(client_fd, resolved_path, raw_path);
        }
    } else {
        int fd = open(resolved_path, O_RDONLY);
        if (fd == -1) return;

        dprintf(client_fd,
                "HTTP/1.1 200 OK\r\n"
                "Content-Type: %s\r\n"
                "Content-Length: %ld\r\n"
                "Accept-Ranges: bytes\r\n"   // Обязательно для MP4
                "Content-Disposition: attachment\r\n"
                "Connection: keep-alive\r\n"
                "Connection: close\r\n"
                "\r\n", 
                get_mime_type(resolved_path), st.st_size);
        if (send_all_sendfile(client_fd, fd, st.st_size) == -1) {
            perror("sendfile");
        }
        close(fd);
    }
}

// --- Main ---

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <dir> <ip:port>\n", argv[0]);
        return 1;
    }

    char *dir = argv[1];
    char *addr_str = strdup(argv[2]);
    char *ip = strtok(addr_str, ":");
    int port = atoi(strtok(NULL, ":"));
    struct stat st;

    // Пытаемся получить информацию о пути
    if (stat(dir, &st) == -1) {
        if (errno == ENOENT) {
            // Если пути не существует
            printf("Directory '%s' not found. Сreate a directory with files.\n", dir);
            return 1;
        } else {
            // Какая-то другая ошибка (например, нет прав на чтение родительской папки)
            perror("Error checking directory");
            return 1;
        }
    } else {
        // Путь существует, проверяем, что это папка
        if (!S_ISDIR(st.st_mode)) {
            fprintf(stderr, "Error: '%s' exists but is not a directory.\n", dir);
            return 1;
        }
    }

    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr = { .sin_family = AF_INET, .sin_port = htons(port) };
    inet_pton(AF_INET, ip, &addr.sin_addr);

    if (bind(listen_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind"); return 1;
    }
    listen(listen_fd, SOMAXCONN);
    set_nonblocking(listen_fd);

    int epoll_fd = epoll_create1(0);
    struct epoll_event ev = { .events = EPOLLIN, .data.fd = listen_fd };
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listen_fd, &ev);

    struct epoll_event events[MAX_EVENTS];
    while (1) {
        int nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        for (int i = 0; i < nfds; i++) {
            if (events[i].data.fd == listen_fd) {
                int client_fd = accept(listen_fd, NULL, NULL);
                set_nonblocking(client_fd);
                struct epoll_event ev_cli = { .events = EPOLLIN | EPOLLET, .data.fd = client_fd };
                epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &ev_cli);
            } else {
                handle_request(events[i].data.fd, dir);
                close(events[i].data.fd);
            }
        }
    }

    printf("Serving files from: %s\n", dir);
    return 0;
}
