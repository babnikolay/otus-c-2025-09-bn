#include <stdio.h>       // Для printf и fprintf
#include <stdlib.h>      // Для функции system()
#include <unistd.h>      // Для функции usleep()
#include <pthread.h>
#include "logger.h"

// Функция, имитирующая глубокий вызов для демонстрации стека
void deep_function(int depth) {
    if (depth > 0) {
        deep_function(depth - 1);
    } else {
        LOG(LOG_ERROR, "Критическая ошибка на глубине стека! Проверьте backtrace.");
    }
}

// Потоковая функция для проверки thread-safety
void* thread_worker(void* arg) {
    int id = *((int*)arg);
    for (int i = 0; i < 3; i++) {
        LOG(LOG_INFO, "Поток #%d выполняет итерацию %d", id, i);
        usleep(100000); // Небольшая задержка
    }
    return NULL;
}

int main() {
    // 1. Инициализация
    if (log_init("app.log") != 0) {
        fprintf(stderr, "Не удалось открыть файл лога!\n");
        return 1;
    }

    printf("Демонстрация работы логгера запущена. Проверьте файл 'app.log'.\n");

    // 2. Обычные сообщения
    LOG(LOG_INFO, "Приложение успешно инициализировано");
    LOG(LOG_DEBUG, "Отладочная информация");
    LOG(LOG_WARNING, "Предупреждение: низкий уровень заряда воображения");

    // 3. Проверка многопоточности
    pthread_t t1, t2;
    int id1 = 1, id2 = 2;
    pthread_create(&t1, NULL, thread_worker, &id1);
    pthread_create(&t2, NULL, thread_worker, &id2);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    // 4. Проверка LOG_ERROR и стека вызовов
    LOG(LOG_INFO, "Вызываем функцию с глубоким стеком для теста ошибки...");
    deep_function(3);

    // 5. Завершение
    LOG(LOG_INFO, "Завершение работы тестового приложения\n");
    log_close();

    printf("Готово. Содержимое лога:\n----------------\n");
    system("cat app.log");

    return 0;
}
