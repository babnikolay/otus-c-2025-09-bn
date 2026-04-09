#define _XOPEN_SOURCE 700
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
#include <fcntl.h>
#include <syslog.h>

#define CONFIG_FILE "daemon.conf"
#define MAX_PATH 256
#define PID_FILE "/tmp/file_size_daemon.pid"

// Глобальные настройки
char file_to_watch[MAX_PATH] = "/tmp/target_file.txt";
char current_socket_path[MAX_PATH] = "/tmp/file_size_daemon.sock";
volatile sig_atomic_t reload_requested = 0;
volatile sig_atomic_t running = 1;

void handle_sighup(int sig) { (void)sig; reload_requested = 1; }
void handle_sigterm(int sig) { (void)sig; running = 0; }

int check_and_write_pid() {
    int fd = open(PID_FILE, O_RDWR | O_CREAT, 0644);
    if (fd == -1) return -1;

    struct flock fl = {.l_type = F_WRLCK, .l_whence = SEEK_SET, .l_start = 0, .l_len = 0};
    if (fcntl(fd, F_SETLK, &fl) == -1) {
        close(fd);
        return -1;
    }

    ftruncate(fd, 0);
    char buf[16];
    snprintf(buf, sizeof(buf), "%d\n", getpid());
    write(fd, buf, strlen(buf));
    return fd; // Возвращаем дескриптор, чтобы держать блокировку
}

void remove_pid_file() {
    unlink(PID_FILE);
}

void trim(char *s) {
    char *p = s;
    int l = strlen(p);
    while(l > 0 && isspace((unsigned char)p[l - 1])) p[--l] = 0;
    while(*p && isspace((unsigned char)*p)) { p++; l--; }
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

    unlink(path);
    if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) == -1 || listen(fd, 5) == -1) {
        close(fd);
        return -1;
    }
    return fd;
}

void daemonize() {
    pid_t pid = fork();
    if (pid < 0) exit(EXIT_FAILURE);
    if (pid > 0) exit(EXIT_SUCCESS);
    if (setsid() < 0) exit(EXIT_FAILURE);
    signal(SIGCHLD, SIG_IGN);
    pid = fork();
    if (pid < 0) exit(EXIT_FAILURE);
    if (pid > 0) exit(EXIT_SUCCESS);
    umask(0);
    chdir("./");
    for (int x = sysconf(_SC_OPEN_MAX); x >= 0; x--) close(x);
    open("/dev/null", O_RDWR); dup(0); dup(0);
}

int main(int argc, char *argv[]) {
    int server_fd = -1;
    int pid_fd = -1;

    openlog("file_size_daemon", LOG_PID, LOG_DAEMON);
    load_config(file_to_watch, current_socket_path);

    // Настройка сигналов
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handle_sighup;
    sigaction(SIGHUP, &sa, NULL);

    sa.sa_handler = handle_sigterm;
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGINT, &sa, NULL);

    if (argc > 1 && strcmp(argv[1], "-d") == 0) daemonize();

    pid_fd = check_and_write_pid();
    if (pid_fd < 0) {
        syslog(LOG_ERR, "Daemon already running or PID file error");
        closelog();
        return EXIT_FAILURE;
    }

recreate_socket:
    server_fd = create_socket(current_socket_path);
    if (server_fd == -1) {
        syslog(LOG_ERR, "Failed to create socket %s: %s", current_socket_path, strerror(errno));
        goto exit_fail;
    }

    syslog(LOG_INFO, "Daemon started. Watching: %s", file_to_watch);

    while (running) {
        int client_fd = accept(server_fd, NULL, NULL);

        if (client_fd < 0) {
            if (errno == EINTR) {
                if (!running) break;
                if (reload_requested) {
                    char n_f[MAX_PATH], n_s[MAX_PATH];
                    strcpy(n_f, file_to_watch); strcpy(n_s, current_socket_path);
                    load_config(n_f, n_s);
                    reload_requested = 0;

                    if (strcmp(n_s, current_socket_path) != 0) {
                        syslog(LOG_INFO, "Socket path changed, reconnecting...");
                        close(server_fd);
                        unlink(current_socket_path);
                        strcpy(current_socket_path, n_s);
                        strcpy(file_to_watch, n_f);
                        goto recreate_socket;
                    }
                    strcpy(file_to_watch, n_f);
                    syslog(LOG_INFO, "Config reloaded");
                }
                continue;
            }
            continue;
        }

        struct stat st;
        char resp[512];
        if (stat(file_to_watch, &st) == 0)
            snprintf(resp, sizeof(resp), "File: %s, Size: %ld\n", file_to_watch, st.st_size);
        else
            snprintf(resp, sizeof(resp), "Error accessing file: %s\n", strerror(errno));

        send(client_fd, resp, strlen(resp), 0);
        close(client_fd);
    }

    syslog(LOG_INFO, "Shutting down");
    if (server_fd != -1) { close(server_fd); unlink(current_socket_path); }
    if (pid_fd != -1) close(pid_fd);
    remove_pid_file();
    closelog();
    return EXIT_SUCCESS;

exit_fail:
    if (server_fd != -1) { close(server_fd); unlink(current_socket_path); }
    if (pid_fd != -1) close(pid_fd);
    remove_pid_file();
    closelog();
    return EXIT_FAILURE;
}
