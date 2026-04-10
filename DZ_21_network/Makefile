# Переменные компилятора
CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -std=c11 -O2
TARGET = ascii_art
SRC = ascii_art.c

# Популярные шрифты для теста
TEST_FONTS = shadow slant bubble

.PHONY: all clean install uninstall test

# Цель по умолчанию
all: $(TARGET)

# Сборка бинарного файла
$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

# Очистка временных файлов
clean:
	rm -f $(TARGET)

# Автоматическое тестирование связи и шрифтов
test: $(TARGET)
	@echo "--- Запуск тестов связи с telehack.com ---"
	@for font in $(TEST_FONTS); do \
		echo "\nПроверка шрифта: $$font"; \
		./$(TARGET) $$font "Test $$font"; \
		sleep 2; \
	done
	@echo "\n--- Тестирование завершено ---"
