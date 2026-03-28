# Настройки компиляторов
AS = nasm
CC = gcc
ASFLAGS = -g -felf64
CFLAGS = -std=c11 -Wall -Wextra -g
LDFLAGS = -no-pie -Wl,-z,noexecstack

# Имена исполняемых файлов
ASM_TARGET = asm-prog
C_TARGET = c-prog

# Список всех целей
all: $(ASM_TARGET) $(C_TARGET)

# Сборка ассемблерной версии
$(ASM_TARGET): main_asm.o
	$(CC) $(LDFLAGS) $^ -o $@

main_asm.o: main.asm
	$(AS) $(ASFLAGS) $< -o $@

# Сборка Си версии
$(C_TARGET): main_c.o
	$(CC) $(CFLAGS) $^ -o $@

main_c.o: main_c.c
	$(CC) $(CFLAGS) -c $< -o $@

# Очистка
clean:
	rm -f $(ASM_TARGET) $(C_TARGET) *.o core

.PHONY: clean all
