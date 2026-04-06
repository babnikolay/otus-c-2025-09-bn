#include "logger.h"

void some_function() {
    LOG(LOG_ERROR, "Критическая ошибка: %s", "база данных недоступна");
}

int main() {
    if (log_init("app.log") != 0) return 1;

    LOG(LOG_INFO, "Приложение запущено");
    LOG(LOG_DEBUG, "Тестовая отладка: x = %d", 42);
    
    some_function();

    log_close();
    return 0;
}
