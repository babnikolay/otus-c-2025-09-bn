#include <stdio.h>       // Для printf и fprintf
#include <stdlib.h>      // Для функции system()
#include <unistd.h>      // Для функции usleep()
#include <pthread.h>
#include <sys/wait.h>
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
        exit(EXIT_FAILURE);
    }

    printf("Демонстрация работы логгера запущена. Проверьте файл 'app.log'.\n");

    // Создаем новый процесс
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork failed");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        // --- КОД ДОЧЕРНЕГО ПРОЦЕССА ---
        LOG(LOG_INFO, "[Child] Процесс-потомок запущен (PID: %d)", getpid());
        
        // Демонстрация ошибки и стека в потомке
        deep_function(2);
        
        LOG(LOG_INFO, "[Child] Завершение работы потомка.");
        log_close();
        
        // Используем _exit для немедленного выхода без вызова atexit родителя
        _exit(EXIT_SUCCESS); 
    }
    else {
         // --- КОД РОДИТЕЛЬСКОГО ПРОЦЕССА ---
        LOG(LOG_INFO, "[Parent] Процесс-родитель (PID: %d) создал потомка (PID: %d)", getpid(), pid);

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

        // Ждем завершения дочернего процесса
        wait(NULL);

        // 5. Завершение
        LOG(LOG_INFO, "Завершение работы тестового приложения\n");
        log_close();
    }

    printf("Готово. Содержимое лога:\n----------------\n");
    system("cat app.log");

    exit(EXIT_SUCCESS);
}
