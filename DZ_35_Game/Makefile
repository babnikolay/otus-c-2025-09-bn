# Имя исполняемого файла
TARGET = tetris

# Компилятор
CC = gcc

# Флаги компиляции (согласно вашим требованиям)
# -Wall -Wextra -Wpedantic: строгие предупреждения
# -std=c11: стандарт языка C11
# `sdl2-config --cflags`: автоматически находит пути к заголовочным файлам SDL2
CFLAGS = -Wall -Wextra -Wpedantic -std=c11 $(shell sdl2-config --cflags)

# Библиотеки для линковки
# `sdl2-config --libs`: автоматически подключает необходимые системные библиотеки SDL2
LIBS = $(shell sdl2-config --libs)

# Список объектных файлов
OBJ = tetris.o

# Правило по умолчанию (просто запуск 'make')
all: $(TARGET)

# Сборка исполняемого файла
$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET) $(LIBS)

# Сборка объектных файлов
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Очистка проекта (команда 'make clean')
clean:
	rm -f $(TARGET) $(OBJ)

# Запуск игры после сборки (команда 'make run')
run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run
