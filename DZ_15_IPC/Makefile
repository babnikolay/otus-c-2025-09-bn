# Параметры компилятора
CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -std=c11 -D_XOPEN_SOURCE=700
TARGET = fsd
SRC = daemon.c
CONF_FILE = daemon.conf

# Пути по умолчанию (для создания тестового конфига)
DEFAULT_FILE = /tmp/target_file.txt
DEFAULT_SOCK = /tmp/file_size_daemon.sock

.PHONY: all clean run reload test

# Сборка проекта
all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

# Запуск в обычном режиме
run: $(TARGET)
	./$(TARGET)

# Запуск в режиме демона
daemon: $(TARGET)
	./$(TARGET) -d

# Создание стандартного файла конфигурации, если его нет
config:
	@echo "FILE_PATH=$(DEFAULT_FILE)" > $(CONF_FILE)
	@echo "SOCKET_PATH=$(DEFAULT_SOCK)" >> $(CONF_FILE)
	@echo "Создан файл $(CONF_FILE)"

# Перезагрузка демона (отправка сигнала SIGHUP)
reload:
	@pkill -HUP $(TARGET) && echo "Сигнал SIGHUP отправлен." || echo "Демон не запущен."

stop:
	@pkill -x $(TARGET) && echo "Приложение $(TARGET) остановлено."

# Быстрый тест через netcat (берет путь из конфига)
test:
	@SOCK_PATH=$$(grep SOCKET_PATH daemon.conf | cut -d'=' -f2); \
	nc -U $$SOCK_PATH || echo "Не удалось подключиться к сокету."

# Очистка артефактов сборки
clean:
	rm -f $(TARGET) $(CONF_FILE)
	rm -f *.o
	@echo "Очистка завершена."
