# Компилятор
CC = gcc
# Имя исполняемого файла
TARGET = parser_logs
DIR ?= logs

# Автоматическое определение количества ядер
# Если nproc доступен, берем его, иначе пробуем sysctl (для Mac), иначе ставим 2
THREADS ?= $(shell nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 2)

# Флаги компиляции:
# -Wall, -Wextra: выводить все предупреждения
# -O3: максимальная оптимизация скорости
# -pthread: поддержка многопоточности
# -D_DEFAULT_SOURCE: для использования расширений POSIX (strdup, strndup)
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -O3 -pthread -D_DEFAULT_SOURCE

# Исходные файлы (предполагается, что ваш код в parser_logs.c)
SRCS = parser_logs.c

# Правило по умолчанию
all: $(TARGET)

# Сборка исполняемого файла
$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET)

# Очистка временных файлов
clean:
	@rm -f $(TARGET)
	@echo "Очистка проекта\n"

# Удобный запуск (пример: make run DIR=./logs THREADS=2)
run: $(TARGET)
	@mkdir -p $(DIR)
	@./$(TARGET) $(DIR) $(THREADS)

.PHONY: all clean run
