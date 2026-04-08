# Переменные для компилятора и флагов
CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=c11 -O3
TARGET = crc32_tool
SRC = crc32_tool.c
FILE ?= file.bin

# Цель по умолчанию (просто запуск make)
all: $(TARGET)

# Сборка исполняемого файла
$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

# Очистка временных файлов
clean:
	rm -f $(TARGET)

# Удобная цель для проверки на файле
test: $(TARGET)
	./$(TARGET) $(FILE)
