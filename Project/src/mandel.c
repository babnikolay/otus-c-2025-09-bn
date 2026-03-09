#include "mandel.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

void help() {
    printf("\n");
    printf("=============== ПОМОЩЬ ===============\n");
    printf("При запуске программы вы можете выбрать:\n");
    printf("1. Прочитать данную инструкцию по запуску (нажать h или H).\n");
    printf("2. Запустить её с параметрами по умолчанию (нажать n, N или Enter).\n");
    printf("3. Изменить параметры по умолчанию (нажать y или Y)\n");
    printf("(в квадратных скобках ниже указаны параметры по умолчанию).\n");
    printf("width       - Ширина окна. [800]\n");
    printf("height      - Высота окна. [800]\n");
    printf("zoom        - Масштабирование изображения. [2.5]\n");
    printf("MAX_ITER    - Максимальное количество итераций, за которое точка выйдет за границы множества. [1000]\n");
    printf("R           - Красный для палитры. [0.8]\n");
    printf("G           - Зелёный для палитры. [0.5]\n");
    printf("B           - Синий для палитры. [2.1]\n");
    printf("\n");
    printf("Сохранение:\n");
    printf("Для сохранения изображения необходимо нажать клавишу 's'\n");
    printf("После чего, ввести размеры окна в пикселях и разрешение в точках на дюйм, \n");
    printf("либо нажать на Enter, чтобы оставить значения по умолчанию.\n");
    printf("Начальная палитра вокруг множества задаётся в RGB в вещественном виде.\n");
    printf("Цвет палитры внутри множества не меняется и всегда чёрный.\n");
    printf("\n");
    printf("Перемещение:\n");
    printf("Для перемещения по изображению можно использовать клавиши: вверх/вниз/влево/вправо.\n");
    printf("Для приближения/удаления изображения можно использовать клавиши [+] / [-]\n");
    printf("Кроме того, клик по изображению левой клавишей 'мыши' приближает изображение,\n");
    printf("а клик правой клавишей 'мыши' - отдаляет изображение. Изображение центрируется по нажатой точке.\n");
    printf("======================================\n");
}

// Функция проверяет, является ли строка числом (целым или с точкой)
int is_valid_number(const char *str) {
    int dot_count = 0;
    if (*str == '\n' || *str == '\0') return 0;
    
    for (int i = 0; str[i] != '\0' && str[i] != '\n'; i++) {
        if (isdigit(str[i])) continue;
        if (str[i] == '.' && dot_count == 0) {
            dot_count++;
            continue;
        }
        return 0; // Нашли недопустимый символ
    }
    return 1;
}

// Функция для безопасного получения числа от пользователя
void get_input(const char *prompt, void *variable, const char *type, const char *default_val) {
    char buffer[100];
    while (1) {
        printf("%s [%s]: ", prompt, default_val);

        if (!fgets(buffer, sizeof(buffer), stdin)) continue;

        // Удаление символа новой строки
        buffer[strcspn(buffer, "\n")] = '\0';

        // Проверка на пустой ввод - если пользователь нажал Enter
        if (strlen(buffer) == 0) {
            // Если в буфере пусто, используем значение по умолчанию
            strcpy(buffer, default_val);
            break;
        }

        if (strcmp(type, "int") == 0) {
            if (is_valid_number(buffer)) {
                *(int *)variable = atoi(buffer);
                break;
            }
        } else if (strcmp(type, "double") == 0) {
            if (is_valid_number(buffer)) {
                *(double *)variable = atof(buffer);
                break;
            }
        }

        printf("Ошибка! Введите число или нажмите Enter для выбора [%s].\n", default_val);
    }
}

// Вычисления цвета вынесены для универсальности
uint32_t compute_pixel(int x, int y, int W, int H, double zoom, double cx, double cy, 
                        int max_iter, double R, double G, double B) {
    // Логарифмические константы - используется для оптимизации log(log(sqrt(z)))
    const double inv_log2 = 1.0 / log(2.0);
    const double log_05 = log(0.5);

    double ca = cx + (x - W / 2.5) * (zoom / W);
    double cb = cy + (y - H / 2.5) * (zoom / W);
    double a = 0, b = 0, a2 = 0, b2 = 0;
    int iter = 0;

    while (a2 + b2 <= 4 && iter < max_iter) {
        b = 2 * a * b + cb;
        a = a2 - b2 + ca;
        a2 = a * a;
        b2 = b * b;
        iter++;
    }

    if (iter == max_iter)
        return 0xFF000000;

    /*
    Алгоритм сглаживания: mu = n + 1 - log(log|z|) / log(2)
    Формула mu: Вычисляет "дробную итерацию". 
    Теперь iter — это не целое число, а вещественное, что позволяет цвету меняться непрерывно.
    Вместо log(log(sqrt(a² + b²))) можно использовать свойства логарифмов, чтобы убрать корень:
    sqrt(z) = z^0.5
    log(z^0.5) = 0.5 * log(z)
    log(0.5 * log(z)) = log(0.5) + log(log(z))
    */
    double r2 = a2 + b2;
    double mu = iter + 1 - ((log_05 + log(log(r2))) * inv_log2);

    /*
    Создаем цикличный цветовой градиент на основе синусов sin(frequency * t + phase) для плавных цветовых переходов
    Это классический способ создать бесконечную "радугу", где фазовый сдвиг (числа 0.8, 0.5, 2.1) определяет баланс цветов
    Подбирая коэффициенты (0.8, 0.5, 2.1), можно менять палитру
    */
    uint8_t r = (uint8_t)(sin(0.1 * mu + R) * 127 + 128);
    uint8_t g = (uint8_t)(sin(0.1 * mu + G) * 127 + 128);
    uint8_t bl = (uint8_t)(sin(0.1 * mu + B) * 127 + 128);

    return (0xFF000000) | (r << 16) | (g << 8) | bl;
}

// Вспомогательная функция для очистки буфера после scanf
void clear_stdin() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

int get_int_default(const char *prompt, int default_val) {
    char buffer[64];
    printf("%s [%d]: ", prompt, default_val);
    
    if (fgets(buffer, sizeof(buffer), stdin) && buffer[0] != '\n') {
        return atoi(buffer); // Возвращаем введенное число
    }
    return default_val; // Возвращаем по умолчанию, если нажат Enter
}

void save_png_with_dpi(double zoom, double cx, double cy, int iter, double R, double G, double B) {

    printf("\n--- ПАРАМЕТРЫ СОХРАНЕНИЯ ---\n");

    // Ввод с дефолтными значениями
    int w = get_int_default("Введите ширину", 800);
    int h = get_int_default("Введите высоту", 800);
    int dpi = get_int_default("Введите DPI", 300);

    // Валидация (защита от отрицательных чисел и 0)
    if (w <= 0) w = 800;
    if (h <= 0) h = 800;
    if (dpi <= 0) dpi = 300;

    // Расчет размера в см
    double w_cm = (double)w / dpi * 2.54;
    double h_cm = (double)h / dpi * 2.54;

    printf("\n--- ИТОГОВЫЙ РАЗМЕР ПЕЧАТИ ---\n");
    printf("\nИтоговые параметры: %dx%d px, %d DPI\n", w, h, dpi);
    printf("Размер при печати: %.2f x %.2f см\n", w_cm, h_cm);
    printf("------------------------------\n");

    if (w_cm > 200.0 || h_cm > 200.0) {
        printf("Предупреждение: Размер печати превышает 2 метра. Продолжить? (y/n): ");
        char confirm;
        if (scanf(" %c", &confirm) != 1) {
            // Если возникла ошибка чтения, очищаем буфер и выходим
            clear_stdin();
            return;
        }

        if (confirm != 'y' && confirm != 'Y') return;
    }

    uint32_t *img = malloc((size_t)w * h * sizeof(uint32_t));
    if (!img) {
        printf("Ошибка: Недостаточно памяти для изображения такого размера!\n");
        return;
    }

    printf("Рендеринг %dx%d... ", w, h);
    fflush(stdout);

#pragma omp parallel for schedule(dynamic)
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            img[y * w + x] = compute_pixel(x, y, w, h, zoom, cx, cy, iter, R, G, B);
        }
    }

    char fname[256];
    snprintf(fname, sizeof(fname), "mandelbrot_%dx%d_%ddpi.png", w, h, dpi);

    // Записываем файл
    if (stbi_write_png(fname, w, h, 4, img, w * 4)) {
        printf("\nУспешно сохранено: %s\n", fname);
    }
    else {
        printf("\nОшибка при сохранении!\n");
    }

    free(img);
}

// Обычный рендер для окна
void render(SDL_Renderer *ren, SDL_Texture *tex, uint32_t *pix, ...) {
    double zoom = 2.5, cx = -1.0, cy = -0.3;
    int max_iter = 1000, W = 800, H = 800;
    double R = 0.8, G = 0.5, B = 2.1;

    va_list args;
    va_start(args, pix);
    RenderParam p;
    while ((p = va_arg(args, RenderParam)) != RENDER_END) {
        if (p == RENDER_ZOOM)
            zoom = va_arg(args, double);
        else if (p == RENDER_CX)
            cx = va_arg(args, double);
        else if (p == RENDER_CY)
            cy = va_arg(args, double);
        else if (p == RENDER_ITER)
            max_iter = va_arg(args, int);
        else if (p == RENDER_WIDTH)
            W = va_arg(args, int);
        else if (p == RENDER_HEIGHT)
            H = va_arg(args, int);
        else if (p == RENDER_R)
            R = va_arg(args, double);
        else if (p == RENDER_G)
            G = va_arg(args, double);
        else if (p == RENDER_B)
            B = va_arg(args, double);
    }
    va_end(args);

#pragma omp parallel for schedule(dynamic)
    for (int y = 0; y < H; y++)
        for (int x = 0; x < W; x++)
            pix[y * W + x] = compute_pixel(x, y, W, H, zoom, cx, cy, max_iter, R, G, B);

    SDL_UpdateTexture(tex, NULL, pix, W * 4);
    SDL_RenderCopy(ren, tex, NULL, NULL);
    SDL_RenderPresent(ren);
}
