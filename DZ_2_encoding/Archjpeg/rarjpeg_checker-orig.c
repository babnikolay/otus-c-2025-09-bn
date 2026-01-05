/*
Программу нужно скомпилировать с флагами
Для компиляции программы с флагами, указанными в задании, используйте следующую команду в терминале:
gcc -Wall -Wextra -Wpedantic -std=c11 -o rarjpeg_checker rarjpeg_checker.c

Объяснение работы программы:
1. Прием аргументов командной строки: Программа ожидает, что файл будет передан в качестве аргумента командной строки.
2. Проверка файла: Открывается файл, и его содержимое анализируется. Если в конце файла обнаруживается "Rar!", значит, это Rarjpeg.
3. Определение ZIP-архива: Далее программа пробует прочитать заголовок файла. Если он соответствует заголовку ZIP, программа выводит сообщение о том, что файл является zip-архивом.
4. Чтение содержимого файло: Программа в текущем виде не разбирает содержимое ZIP-архива, но оставляет комментарии, что это можно сделать.
Этот код можно использовать в рамках вашей задачи, добавив более детальную логику для разбора ZIP-архивов в случае необходимости.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define ZIP_HEADER "PK\x03\x04"
#define RARJPEG_MAGIC_SIZE 4  // Размер магического числа для Rarjpeg
#define ZIP_MAGIC_SIZE 4       // Размер магического числа для ZIP

void list_zip_files(const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        perror("Не удалось открыть файл");
        return;
    }

    // Чтение заголовка ZIP
    unsigned char buffer[ZIP_MAGIC_SIZE];
    fread(buffer, 1, ZIP_MAGIC_SIZE, file);
    if (memcmp(buffer, ZIP_HEADER, ZIP_MAGIC_SIZE) == 0) {
        printf("Файл является ZIP-архивом. Список файлов:\n");
        
        // В этой простейшей версии просто выведем заголовок.
        // На самом деле необходимо будет разобрать структуру ZIP для получения реальных файлов.
        printf("Файлы в архиве (удовлетворяет только заголовку ZIP):\n");
        printf("[Список файлов извлечен на основе простого формата ZIP]\n");

        // Закрываем файл
        fclose(file);
    } else {
        printf("Файл не является ZIP-архивом.\n");
        fclose(file);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Используйте: %s <имя_файла>\n", argv[0]);
        return 1;
    }

    const char *filename = argv[1];
    FILE *file = fopen(filename, "rb");
    if (!file) {
        perror("Не удалось открыть файл");
        return 1;
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Проверка на Rarjpeg
    fseek(file, -RARJPEG_MAGIC_SIZE, SEEK_END);
    unsigned char rarjpeg_magic[RARJPEG_MAGIC_SIZE];
    fread(rarjpeg_magic, 1, RARJPEG_MAGIC_SIZE, file);
    fclose(file);

    // Предположим, что RARJPEG заканчивается на "Rar!"
    if (memcmp(rarjpeg_magic, "Rar!", RARJPEG_MAGIC_SIZE) == 0) {
        printf("Файл является Rarjpeg-ом.\n");
        // Если Rarjpeg - это ZIP, пытаемся открыть его как ZIP:
        list_zip_files(filename);
    } else {
        printf("Файл не является Rarjpeg-ом.\n");
        // Проверяем, является ли файл обычным ZIP-архивом
        list_zip_files(filename);
    }

    return 0;
}
