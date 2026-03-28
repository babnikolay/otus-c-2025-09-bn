#!/bin/bash

# Настройки
MODULE_NAME="lesson_module"      # Имя вашего модуля (без .ko)
MODULE_FILE="${MODULE_NAME}.ko"
BUILD_DIR=$(pwd)/build

# Проверка прав (установка требует root)
# if [ "$EUID" -ne 0 ]; then
#   echo "Ошибка: Запустите скрипт через sudo"
#   exit 1
# fi

echo "--- Шаг 1: Сборка модуля ---"
make clean
make all
if [ $? -ne 0 ]; then
    echo "Ошибка компиляции!"
    exit 1
fi

echo "--- Шаг 2: Удаление старой версии (если есть) ---"
# Выгружаем модуль, если он уже в памяти
if lsmod | grep -q "^${MODULE_NAME}"; then
    echo "Выгрузка старого модуля..."
    rmmod ${MODULE_NAME}
fi

echo "--- Шаг 3: Системная установка (modules_install) ---"
# Устанавливаем в /lib/modules/$(uname -r)/extra/
make modules_install

echo "--- Шаг 4: Обновление зависимостей ---"
depmod -a

echo "--- Шаг 5: Загрузка модуля через modprobe ---"
# modprobe теперь найдет модуль в системных папках
modprobe ${MODULE_NAME}

if [ $? -eq 0 ]; then
    echo "Успех: Модуль '${MODULE_NAME}' загружен и готов к работе."
    echo "Последние сообщения из dmesg:"
    dmesg | tail -n 5
else
    echo "Ошибка при загрузке модуля!"
    exit 1
fi
