#ifndef LOGGER_H
#define LOGGER_H

#include <stdio.h>

typedef enum {
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARNING,
    LOG_ERROR
} log_level_t;

// Инициализация: открытие файла
int log_init(const char *filename);

// Закрытие лог-файла
void log_close(void);

// Основная функция (лучше вызывать через макрос)
void log_message(log_level_t level, const char *file, int line, const char *fmt, ...);

// Макрос для автоматического подстановки файла и строки
#define LOG(level, ...) log_message(level, __FILE__, __LINE__, __VA_ARGS__)

// // Макросы для удобного вызова
// #define LOG_D(fmt, ...) log_message(LOG_DEBUG, __FILE__, __LINE__, ##__VA_ARGS__)
// #define LOG_I(fmt, ...) log_message(LOG_INFO, __FILE__, __LINE__, ##__VA_ARGS__)
// #define LOG_W(fmt, ...) log_message(LOG_WARNING, __FILE__, __LINE__, ##__VA_ARGS__)
// #define LOG_E(fmt, ...) log_message(LOG_ERROR, __FILE__, __LINE__, ##__VA_ARGS__)

#endif
