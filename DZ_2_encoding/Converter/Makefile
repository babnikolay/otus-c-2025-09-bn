# Имя исполняемого файла
TARGET = converter

# Компилятор и флаги
CC = gcc
CFLAGS = -Wall -Wextra -std=c11

# Список исходных файлов (все .c файлы в папке)
SRCS = converter.c
# Превращаем список .c в .o (объектные файлы)
OBJS = $(SRCS:.c=.o)

# Правило по умолчанию (просто запуск 'make')
all: $(TARGET)

# Сборка программы
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

# Правило для объектных файлов
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Очистка мусора
clean:
	rm -f $(OBJS) $(TARGET)

# Пересборка с нуля
rebuild: clean all
