# Настройки компилятора
CC = gcc
INCDIR = include
SRCDIR = src
OBJDIR = obj
# -MMD генерирует .d файлы, -MP добавляет пустые цели для заголовков (защита от ошибок удаления)
CFLAGS = -O3 -std=c11 -Wall -Wextra -Wpedantic -g -MMD -MP -I$(INCDIR)

# Список "главных" файлов программ (те, в которых есть main())
PROG_SRCS = $(wildcard $(SRCDIR)/mandel_*.c)

# Формируем COMMON_SRCS без mandel.c
# Теперь в COMMON_OBJS только то, что реально нужно ВСЕМ (например, utils.o)
COMMON_SRCS = $(filter-out $(PROG_SRCS) $(SRCDIR)/mandel.c, $(wildcard $(SRCDIR)/*.c))
COMMON_OBJS = $(COMMON_SRCS:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

# Формируем имена исполняемых файлов (из src/mandel_bw.c получаем mandel_bw)
TARGETS = $(PROG_SRCS:$(SRCDIR)/%.c=%)

# TARGETS = mandel_bw mandel_color mandel_smooth  mandel_anim mandel_sdl mandel_sdl_mouse

# Указываем дополнительный объектный файл ТОЛЬКО для этой цели
mandel_sdl_mouse: $(OBJDIR)/mandel.o
# --- Специфичные настройки библиотек ---
mandel_smooth: LDLIBS = -lm
mandel_anim: LDLIBS = -lm

mandel_sdl: CFLAGS += -fopenmp
mandel_sdl: LDLIBS = -lSDL2 -lm -fopenmp

mandel_sdl_mouse: CFLAGS += -fopenmp
mandel_sdl_mouse: LDLIBS = -lSDL2 -lm -fopenmp

# Правило по умолчанию (сборка проекта)
all: $(TARGETS)

.DEFAULT_GOAL := all

# СТАТИЧЕСКОЕ ПРАВИЛО:
# Для каждого имени в списке TARGETS (например, mandel_bw):
# Ищем соответствующий объектный файл (obj/mandel_bw.o) 
# и добавляем к нему все общие объекты (COMMON_OBJS)
$(TARGETS): %: $(OBJDIR)/%.o $(COMMON_OBJS)
	$(CC) $^ -o $@ $(LDLIBS)

# Сборка исполняемого файла
$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET)

# Компиляция .c из src/ в .o в obj/
$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Создание папки obj, если её нет
$(OBJDIR):
	mkdir -p $(OBJDIR)

# Подключаем сгенерированные файлы зависимостей
# Тире в начале игнорирует ошибки, если файлов еще нет (при первой сборке)
# -include $(DEPS)
# -include $(OBJDIR)/*.d

# Очистка временных файлов
clean:
# 	rm -f $(OBJS) $(DEPS) $(TARGET)
	rm -rf $(OBJDIR) $(TARGETS)

.PHONY: all clean
