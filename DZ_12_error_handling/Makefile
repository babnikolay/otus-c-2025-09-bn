# Настройки проекта
LIB_NAME = liblogger.a
TARGET   = app
CC       = gcc

# Директории
SRC_DIR  = src
INC_DIR  = include
OBJ_DIR  = obj
DIST_DIR = dist
BIN_DIR  = $(DIST_DIR)/bin

# Флаги
# -rdynamic и -pthread важны для работы стека и потоков
CFLAGS   = -std=c11 -Wall -Wextra -Wpedantic -g -MMD -MP -I$(INC_DIR) -D_DEFAULT_SOURCE
LDFLAGS  = -rdynamic -pthread

# Объекты
LIB_OBJ  = $(OBJ_DIR)/logger.o
TEST_OBJ = $(OBJ_DIR)/test.o

# Цвета
GREEN  := \033[0;32m
RED    := \033[0;31m
CYAN   := \033[0;36m
YELLOW := \033[0;33m
NC     := \033[0m

LIB_NAME = liblogger.a
LIB_OBJ  = $(OBJ_DIR)/logger.o
TEST_OBJ = $(OBJ_DIR)/test.o

# Основная цель: собирает библиотеку и тест
all: $(LIB_NAME) $(BIN_DIR)/$(TARGET)

# 1. Создание статической библиотеки
$(LIB_NAME): $(LIB_OBJ)
	@echo "$(CYAN)Creating static library:$(NC) $@"
	@ar rcs $@ $^

# 2. Сборка тестового приложения (линковка объекта теста с библиотекой)
$(BIN_DIR)/$(TARGET): $(TEST_OBJ) $(LIB_NAME)
	@mkdir -p $(BIN_DIR)
	@echo "$(CYAN)Linking application:$(NC) $@"
	@$(CC) $(TEST_OBJ) -L. -llogger -o $@ $(LDFLAGS)
	@echo "$(GREEN)--------------------------------------$(NC)"
	@echo "$(GREEN)Сборка завершена успешно:$(NC) $(YELLOW)$@$(NC)"

# Компиляция объектных файлов
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	@echo "$(CYAN)Compiling:$(NC) $<"
	@$(CC) $(CFLAGS) -c $< -o $@

# Очистка
clean:
	@rm -rf $(OBJ_DIR) $(DIST_DIR) $(LIB_NAME) app.log
	@echo "$(RED)Проект очищен.$(NC)"

# Запуск
run: all
	@echo "$(YELLOW)Запуск теста...$(NC)"
	@./$(BIN_DIR)/$(TARGET)

.PHONY: all clean run

# Подключение зависимостей
-include $(LIB_OBJ:.o=.d) $(TEST_OBJ:.o=.d)
