# Переменные
CC = gcc
CFLAGS =  -Wall -Wextra -Wpedantic -std=c11 -O3 -march=native
TARGET = http_server
FILES_DIR = files
SRC = server.c
TEST_DIR ?= ./www
INDEX_FILE = $(TEST_DIR)/index.html
ADDR ?= 127.0.0.1
PORT ?= 8080
ARGS = $(TEST_DIR) $(ADDR):$(PORT)

# Цель по умолчанию
all: $(TARGET) setup

# Создание тестовой инфраструктуры
setup:
	@mkdir -p $(TEST_DIR)
	@if [ ! -f $(INDEX_FILE) ]; then \
		echo "<html><body><h1>It works!</h1><p><h3>Served by epoll server<h3></p></body></html>" > $(INDEX_FILE); \
	fi

# Сборка
$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

# Остановка сервера: ищем только по точному имени исполняемого файла
stop:
	@pkill -x $(TARGET) || true
	@echo "Сервер остановлен."

# Запуск в фоновом режиме с небольшой задержкой для проверки
run: all
	@cp -r $(FILES_DIR) $(TEST_DIR)
	@./$(TARGET) $(ARGS) & \
	echo "Сервер запущен на $(ARGS) (PID: $$!)"
	@rm -f $(INDEX_FILE)

# Очистка
clean: stop
	@rm -f $(TARGET)
	@rm -rf $(TEST_DIR)
	@echo "Очищено."

.PHONY: all setup stop run clean
