# Настройки компилятора и путей
CC = gcc
INCDIR = include
SRCDIR = src
OBJDIR = obj
BINDIR = dist/bin
DIR = sdl_mouse

# --- Проверка системных зависимостей ---
PKGS := sdl2 pkg-config
# Проверяем, установлены ли пакеты через pkg-config
CHECK_PKGS := $(shell pkg-config --exists $(PKGS) && echo yes || echo no)
# Проверяем поддержку OpenMP компилятором
CHECK_OMP  := $(shell echo | $(CC) -fopenmp -x c -E - > /dev/null 2>&1 && echo yes || echo no)

# Базовые флаги
CFLAGS = -O3 -std=c11 -Wall -Wextra -Wpedantic -g -MMD -MP -I$(INCDIR)

# Списки файлов
PROG_SRCS   := $(wildcard $(SRCDIR)/mandelbrot*.c)
# Все .c в src, кроме программных и mandel.c
COMMON_SRCS := $(filter-out $(PROG_SRCS) $(SRCDIR)/mandel.c, $(wildcard $(SRCDIR)/*.c))
COMMON_OBJS := $(COMMON_SRCS:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

# Имена исполняемых файлов (например, bin/mandelbrot_sdl)
TARGETS     := $(PROG_SRCS:$(SRCDIR)/%.c=$(BINDIR)/%)

# Цвета
GREEN  := \033[0;32m
RED    := \033[0;31m
CYAN   := \033[0;36m
YELLOW := \033[0;33m
NC     := \033[0m

all: $(OBJDIR) $(BINDIR)
	@# Если чего-то не хватает — устанавливаем и ПЕРЕЗАПУСКАЕМ everything
	@if [ "$(CHECK_PKGS)" = "no" ] || [ "$(CHECK_OMP)" = "no" ]; then \
		printf "$(YELLOW)[WARN]$(NC) Зависимости не найдены или не видны компилятору!\n"; \
		sudo apt-get update && sudo apt-get install -y libsdl2-2.0-0 libsdl2-dev pkg-config libomp-dev; \
		sudo apt-get install -y libsdl2-image-dev libsdl2-image-2.0-0; \
		sudo ldconfig; \
		printf "$(GREEN)[OK]$(NC) Пакеты обновлены. Запуск сборки...\n"; \
		$(MAKE) build_process --no-print-directory; \
	else \
		printf "$(GREEN)[OK]$(NC) Зависимости (SDL2, OpenMP) найдены.\n"; \
		$(MAKE) build_process --no-print-directory; \
	fi

# Выносим процесс сборки в отдельную цель, чтобы вызывать её и при успехе, и после установки
build_process:
	@START=$$(date +%s.%N); \
	printf "\n$(CYAN)--- Сборка запущена: %s ---$(NC)\n\n" "$$(date +'%H:%M:%S')"; \
	if $(MAKE) $(TARGETS) --no-print-directory; then \
		END=$$(date +%s.%N); \
		DIFF=$$(python3 -c "print(round($$END - $$START, 3))" 2>/dev/null || echo "0"); \
		printf "\n$(GREEN)--- Сборка УСПЕШНА (Время: %s сек) ---$(NC)\n\n" "$$DIFF"; \
	else \
		printf "\n$(RED)--- Сборка ПРЕРВАНА ---$(NC)\n\n"; exit 1; \
	fi

# Дефолтные аргументы для каждой программы
mandelbrot_anim:      ARGS = 2 1000 2.5 0.99 -1.768778833 -0.001738974 0.3 0.0 2.1 4.2
mandelbrot_color:     ARGS = 2
mandelbrot_sdl:       ARGS = 1000 800
mandelbrot_smooth:    ARGS = 3.5 0.1 0.0 2.1 3.2

# Правила для сборки конкретных программ по имени (без пути)
# Позволяет запускать 'make имя_программы' вместо 'make dist/bin/имя_программы'
$(notdir $(TARGETS)): %: $(BINDIR)/%
	@printf "$(CYAN)[RUN]$(NC)  Запуск %s с аргументами: $(YELLOW)%s$(NC)...\n" "$<" "$(ARGS)"
	@./$< $(ARGS)

# Запрет make удалять .o файлы при ошибках
.PRECIOUS: $(OBJDIR)/%.o

# Специфичные настройки библиотек (должны быть ПЕРЕД правилом линковки)
$(BINDIR)/mandelbrot_smooth: 	LDLIBS = -lm
$(BINDIR)/mandelbrot_anim:   	LDLIBS = -lm
$(BINDIR)/mandelbrot_sdl:       CFLAGS += -fopenmp
$(BINDIR)/mandelbrot_sdl:       LDLIBS = -lSDL2 -lm -fopenmp
$(BINDIR)/mandelbrot_sdl_mouse: CFLAGS += -fopenmp
$(BINDIR)/mandelbrot_sdl_mouse: LDLIBS = -lSDL2 -lm -fopenmp
$(BINDIR)/mandelbrot_sdl_mouse: $(OBJDIR)/mandel.o

# 1. Правило компиляции объектных файлов
$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	@echo "$(YELLOW)[CC]$(NC)  $<"
	@$(CC) $(CFLAGS) -c $< -o $@

# 2. Правило линковки (ИСПРАВЛЕНО: убрана неоднозначность bin/%)
$(BINDIR)/%: $(OBJDIR)/%.o $(COMMON_OBJS) | $(BINDIR)
	@echo "$(GREEN)[LD]$(NC)  $@"
	@$(CC) $(CFLAGS) $^ -o $@ $(LDLIBS)

# Создание директорий
$(OBJDIR) $(BINDIR):
	@mkdir -p $@

clean:
	@echo "$(RED)[RM]$(NC)  Очистка проекта (Makefile + CMake)..."
	# 1. Удаляем папки Makefile
	@rm -rf $(OBJDIR) $(BINDIR) $(DIR)
	# 2. Удаляем стандартные папки установки и сборки CMake (если есть)
	@rm -rf build/ dist/
	# 3. Удаляем мусор, если CMake запускали прямо в корне (in-source build)
	@rm -rf CMakeFiles/ CMakeCache.txt cmake_install.cmake Makefile.cmake
	@echo "$(GREEN)[OK]$(NC)  Все временные файлы удалены."


# -include $(OBJDIR)/*.d

.PHONY: all clean build_process $(notdir $(TARGETS))
