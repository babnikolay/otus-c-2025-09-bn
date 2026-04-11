# Имя исполняемого файла
TARGET = zipjpeg_checker

# Компилятор и флаги
CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -O2

# Список исходных файлов
SRCS = zipjpeg_checker.c

# Правило по умолчанию
all: $(TARGET)

# Сборка проекта
$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET)

# Очистка временных файлов
clean:
	rm -f $(TARGET)

# Запуск с тестовым файлом (например, zippng.png)
run: $(TARGET)
	./$(TARGET) zippng.png
