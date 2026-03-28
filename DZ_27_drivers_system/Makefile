MODULE_NAME := bn_device
TARGET_TEST = module_test

# Имя файла с исходным кодом в папке src (без расширения .c)
SRC_FILENAME := module_ring

# Пути
PWD       	:= $(shell pwd)
SRC_DIR   	:= $(PWD)/src
OBJ_DIR 	:= $(PWD)/obj
BUILD_DIR	:= $(PWD)/build
KERNELDIR 	:= /lib/modules/$(shell uname -r)/build

# Настройки
CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -I$(SRC_DIR)  # Добавьте пути к заголовкам, если нужно

TEST_OBJS := $(OBJ_DIR)/$(TARGET_TEST).o

# Указываем Kbuild, что модуль состоит из объекта в папке src
obj-m := $(MODULE_NAME).o
$(MODULE_NAME)-y := ../src/$(SRC_FILENAME).o

.PHONY: all clean load unload test install

all:
	@mkdir -p $(BUILD_DIR)
	@echo "# Копируем Makefile в build, чтобы Kbuild видел инструкции внутри M="
	@cp $(SRC_DIR)/$(SRC_FILENAME).c $(BUILD_DIR)/
	# Важно: kbuild ищет Makefile или Kbuild файл в директории M
	# Создаем временный Kbuild файл прямо в build
	@echo "obj-m := $(MODULE_NAME).o" > $(BUILD_DIR)/Kbuild
	@echo "$(MODULE_NAME)-y := $(SRC_FILENAME).o" >> $(BUILD_DIR)/Kbuild 
	@$(MAKE) -C $(KERNELDIR) M=$(BUILD_DIR) modules

# Изменяем правило сборки теста, добавляя зависимость от объектного файла в папке obj
# Правило сборки исполняемого файла теперь зависит от всех найденных объектов
$(TARGET_TEST): $(TEST_OBJS)
	@echo "--- Линковка теста ---"
	@$(CC) $(CFLAGS) $< -o $@

# Добавляем отдельное правило для компиляции .c -> .o именно в папку obj
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	@echo "--- Компиляция $< в $@ ---"
	@$(CC) $(CFLAGS) -c $< -o $@

# Убеждаемся, что папка создается
$(OBJ_DIR):
	@mkdir -p $(OBJ_DIR)


deploy: modules_install
	@echo "--- Загрузка модуля через modprobe ---"
	@sudo modprobe $(MODULE_NAME)
	@$(MAKE) check_dev

# Проверка наличия файла устройства
check_dev:
	@echo "--- Проверка файла устройства /dev/$(MODULE_NAME) ---"
	@if [ -e /dev/$(MODULE_NAME) ]; then \
		echo "Успех: файл /dev/$(MODULE_NAME) создан."; \
		ls -l /dev/$(MODULE_NAME); \
	else \
		echo "Ошибка: файл /dev/$(MODULE_NAME) НЕ найден!"; \
		echo "Проверьте вызов device_create() в коде драйвера."; \
		dmesg | tail -n 10; \
		exit 1; \
	fi

install modules_install: all
	# При установке также важно указать src=$(SRC_DIR), чтобы найти .ko в build
	@echo
	@echo "--- Установка модуля ---"
	@sudo $(MAKE) -C $(KERNELDIR) M=$(BUILD_DIR) src=$(PWD) modules_install
	@sudo depmod -a

# Цель для загрузки модуля
load: all
	@echo
	@echo "--- Очистка логов и перезагрузка модуля ---"
	@sudo dmesg -C  # Очищаем старые логи ядра
	@-sudo rmmod $(MODULE_NAME) 2>/dev/null || true
	sudo insmod $(BUILD_DIR)/$(MODULE_NAME).ko

	@echo "--- Проверка /proc/devices ---"
	@cat /proc/devices | grep $(MODULE_NAME) || echo "ВНИМАНИЕ: Имя $(MODULE_NAME) не найдено в /proc/devices!"
	
# Цель для ручного создания ноды устройства, если драйвер не использует class_create
	@echo "--- Создание файла устройства (mknod) ---"
	@echo "Удаляем старый файл, если он остался"
	@sudo rm -f /dev/$(MODULE_NAME)
	@echo "Ищем мажорный номер в /proc/devices и создаем ноду (минорный обычно 0)"
	@MAJOR=$$(awk '$$2=="$(MODULE_NAME)" {print $$1}' /proc/devices); \
	if [ -n "$$MAJOR" ]; then \
		sudo mknod /dev/$(MODULE_NAME) c $$MAJOR 0; \
		sudo chmod 666 /dev/$(MODULE_NAME); \
		echo "Устройство /dev/$(MODULE_NAME) создано (Major: $$MAJOR)\n"; \
	else \
		echo "Ошибка: Не удалось найти $(MODULE_NAME) в /proc/devices"; \
		echo "Не удалось создать ноду. Проверьте имя в register_chrdev()"; \
		dmesg | tail -n 20; \
		exit 1; \
	fi

# Цель для выгрузки модуля
unload:
	@if lsmod | grep -q "^$(MODULE_NAME)"; then \
		sudo rmmod $(MODULE_NAME); \
		echo "Модуль $(MODULE_NAME) выгружен."; \
	fi
	@sudo rm -f /dev/$(MODULE_NAME)

# Цель для запуска теста
test: $(TARGET_TEST) load
	@echo
	@echo "----- Запуск теста... -----"
	@sudo ./$(TARGET_TEST) || (echo "Тест завершился с ошибкой!"; $(MAKE) unload; exit 1)
	@echo
	@echo "--- Логи ядра (dmesg) ---"
	@dmesg | tail -n 20
	@$(MAKE) unload

clean:
	@if [ -d $(BUILD_DIR) ]; then \
		$(MAKE) -C $(KERNELDIR) M=$(BUILD_DIR) clean; \
	fi
	rm -rf $(BUILD_DIR) $(OBJ_DIR) $(TARGET_TEST)
	rm -f $(SRC_DIR)/*.o $(SRC_DIR)/.*.cmd
