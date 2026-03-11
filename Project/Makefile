# Настройки компилятора
CC = gcc
INCDIR = include
SRCDIR = src
OBJDIR = obj
DIR = sdl_mouse
# -MMD генерирует .d файлы, -MP добавляет пустые цели для заголовков (защита от ошибок удаления)
CFLAGS = -O3 -std=c11 -Wall -Wextra -Wpedantic -g -MMD -MP -I$(INCDIR)

# Список "главных" файлов программ (те, в которых есть main())
PROG_SRCS = $(wildcard $(SRCDIR)/mandelbrot*.c)

# Формируем COMMON_SRCS без mandel.c
# Теперь в COMMON_OBJS только то, что реально нужно ВСЕМ
COMMON_SRCS = $(filter-out $(PROG_SRCS) $(SRCDIR)/mandel.c, $(wildcard $(SRCDIR)/*.c))
COMMON_OBJS = $(COMMON_SRCS:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

# Формируем имена исполняемых файлов (из src/mandel_bw.c получаем mandel_bw)
TARGETS = $(PROG_SRCS:$(SRCDIR)/%.c=%)

# Указываем дополнительный объектный файл ТОЛЬКО для этой цели
mandelbrot_sdl_mouse: $(OBJDIR)/mandel.o

# --- Специфичные настройки библиотек ---
mandelbrot_smooth: LDLIBS = -lm
mandelbrot_anim: LDLIBS = -lm

mandelbrot_sdl: CFLAGS += -fopenmp
mandelbrot_sdl: LDLIBS = -lSDL2 -lm -fopenmp

mandelbrot_sdl_mouse: CFLAGS += -fopenmp
mandelbrot_sdl_mouse: LDLIBS = -lSDL2 -lm -fopenmp

# Правило по умолчанию (сборка проекта)
# Цвета
GREEN  := \033[0;32m
RED    := \033[0;31m
CYAN   := \033[0;36m
YELLOW := \033[0;33m
NC     := \033[0m

all:
	@START=$$(date +%s.%N); \
	printf "\n$(CYAN)--- Сборка запущена: %s ---$(NC)\n\n" "$$(date +'%H:%M:%S')"; \
	if $(MAKE) $(TARGETS) --no-print-directory; then \
		END=$$(date +%s.%N); \
		DIFF=$$(python3 -c "print(round($$END - $$START, 3))" 2>/dev/null || echo "0"); \
		printf "\n$(GREEN)--- Сборка УСПЕШНА: %s (Время: %s сек) ---$(NC)\n\n" "$$(date +'%H:%M:%S')" "$$DIFF"; \
	else \
		printf "\n$(RED)--- Сборка ПРЕРВАНА из-за ошибки в %s ---$(NC)\n\n" "$$(date +'%H:%M:%S')"; \
		exit 1; \
	fi

.DEFAULT_GOAL := all

# Сборка исполняемого файла
$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET)

# Компиляция .c из src/ в .o в obj/
$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	@echo "$(YELLOW)[CC]$(NC)  $<"
	@$(CC) $(CFLAGS) -c $< -o $@
# Чтобы сборка была более информативна, надо убрать символ @
# 	$(CC) $(CFLAGS) -c $< -o $@

# СТАТИЧЕСКОЕ ПРАВИЛО:
# Для каждого имени в списке TARGETS (например, mandel_bw):
# Ищем соответствующий объектный файл (obj/mandel_bw.o) 
# и добавляем к нему все общие объекты (COMMON_OBJS)
$(TARGETS): %: $(OBJDIR)/%.o $(COMMON_OBJS)
	@echo "$(GREEN)[LD]$(NC)  $@"
	@$(CC) $^ -o $@ $(LDLIBS)
# Чтобы сборка была более информативна, надо убрать символ @
# 	$(CC) $^ -o $@ $(LDLIBS)

# Создание папки obj, если её нет
$(OBJDIR):
	mkdir -p $(OBJDIR)

# Очистка временных файлов
clean:
	@echo "$(RED)[RM]$(NC)  Cleaning build files..."
	rm -rf $(OBJDIR) $(TARGETS) $(DIR)

.PHONY: all clean
