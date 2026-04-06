#include "logger.h"
#include <stdarg.h>
#include <time.h>
#include <execinfo.h>
#include <stdlib.h>
#include <pthread.h>

#define BUFFER 64

static FILE *log_file = NULL;
static pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;

static const char* level_strings[] = {
    "DEBUG", "INFO", "WARNING", "ERROR"
};

int log_init(const char *filename) {
    if (!filename) return -1;

    // Блокируем на случай, если кто-то пытается переинициализировать логгер
    pthread_mutex_lock(&log_mutex);

     if (log_file) fclose(log_file); // Закрываем старый файл, если был открыт
    log_file = fopen(filename, "a");
    
    pthread_mutex_unlock(&log_mutex);

    return log_file != NULL ? 0 : -1;
}

void log_close(void) {
    pthread_mutex_lock(&log_mutex);
    if (log_file) {
        fclose(log_file);
        log_file = NULL;
    }
    pthread_mutex_unlock(&log_mutex);
}

static void print_stack_trace(void) {
    void *buffer[BUFFER];
    int nptrs = backtrace(buffer, BUFFER);
    char **strings = backtrace_symbols(buffer, nptrs);
    
    if (strings == NULL) {
        // ОБРАБОТКА NULL: если память не выделилась, выводим хотя бы адреса (без имен)
        fprintf(log_file, "  [!] Stack Trace: out of memory to resolve symbols. Raw addresses:\n");
        for (int i = 0; i < nptrs; i++) {
            fprintf(log_file, "  [%d] %p\n", i, buffer[i]);
        }
    } else {
        fprintf(log_file, "  --- Stack Trace ---\n");
        for (int i = 0; i < nptrs; i++) {
            fprintf(log_file, "  %s\n", strings[i]);
        }
        free(strings);
    }
}

void log_message(log_level_t level, const char *file, int line, const char *fmt, ...) {
    if (!log_file) return;

    // Входим в критическую секцию
    pthread_mutex_lock(&log_mutex);

    // Время записи
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char time_buf[26];

    // Безопасное форматирование времени
    if (strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", t) == 0) {
        time_buf[0] = '\0';
    }

    // Заголовок: [Время] [УРОВЕНЬ] [Файл:Строка]
    fprintf(log_file, "[%s] [%s] [%s:%d]: ", time_buf, level_strings[level], file, line);

    // Само сообщение
    va_list args;
    va_start(args, fmt);
    vfprintf(log_file, fmt, args);
    va_end(args);
    fprintf(log_file, "\n");

    // Вывод стека при ошибке
    if (level == LOG_ERROR) {
        print_stack_trace();
    }

    fflush(log_file);

    // Выходим из критической секции
    pthread_mutex_unlock(&log_mutex);
}
