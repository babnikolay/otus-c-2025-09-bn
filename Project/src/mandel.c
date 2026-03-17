#include "mandel.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

// Color palette[PALETTE_SIZE];

double zoom = 3.0, ca = -0.7, cb = 0.0;
int max_iter = 1000, W = 800, H = 800;
double R = 255.0, G = 150.0, B = 0.0;
// double phase = 0.1; // Сдвиг всей палитры
double degree = 2.0;

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
    printf("R           - Красный для палитры. [0.0]\n");
    printf("G           - Зелёный для палитры. [2.094]\n");
    printf("B           - Синий для палитры. [4.188]\n");
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

Color get_spectral_color (int iter, int max_iter, double modul, double degree, double R, double G, double B) {
    Color color;

    if (iter >= max_iter) return (Color){0, 0, 0}; // Самое множество — черное

    // iter — это ваша итерация (лучше использовать сглаженную дробную итерацию)
    double freq = 0.1; // Скорость смены цветов (чем меньше, тем шире полосы)

    /*
    Алгоритм сглаживания (нормализованное количество итераций): mu = n + 1 - log(log|z|) / log(2)
    Для степеней отличных от 2, формула mu немного меняется: log(log|z|)/log(degree)
    Формула mu: Вычисляет "дробную итерацию".
    Теперь iter — это не целое число, а вещественное, что позволяет цвету меняться непрерывно.
    Вместо log(log(sqrt(a² + b²))) можно использовать свойства логарифмов, чтобы убрать корень:
    sqrt(z) = z^0.5
    log(z^0.5) = 0.5 * log(z)
    log(0.5 * log(z)) = log(0.5) + log(log(z))
    */
    double inv_log_degree = 1.0 / log(degree);
    const double log_05 = log(0.5);
    double log_modul = log(modul);
    double mu = iter + 1 - ((log(log_modul) + log_05) * inv_log_degree);

    color.r = (unsigned char)(sin(freq * mu + R) * 127 + 128);
    color.g = (unsigned char)(sin(freq * mu + G) * 127 + 128); // 2.09 ≈ 2π/3
    color.b = (unsigned char)(sin(freq * mu + B) * 127 + 128); // 4.18 ≈ 4π/3

    // Более «ядовитые» и четкие цвета
    // color.r = (unsigned char)(pow(0.5 * cos(freq * mu + R) + 0.5, 2.0) * 255);
    // color.g = (unsigned char)(pow(0.5 * cos(freq * mu + G) + 0.5, 2.0) * 255);
    // color.b = (unsigned char)(pow(0.5 * cos(freq * mu + B) + 0.5, 2.0) * 255);
    
    return color;
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

// Функция для вычисления цвета
uint32_t compute_pixel(int x, int y, int W, int H, double zoom, double ca, double cb, 
                          int max_iter, double R, double G, double B, double degree) {
    // Центрирование и масштабирование
    double cx = ca + (x - (double)W / 2.0) * (zoom / (double)W);
    double cy = cb + (y - (double)H / 2.0) * (zoom / (double)H);
    
    double zx = 0, zy = 0;
    int iter = 0;
    double r2 = 0;

    while (r2 <= 16 && iter < max_iter) { // Радиус выхода 16 - лучше для высоких степеней
        // Переход в полярные координаты
        double r = sqrt(zx * zx + zy * zy);
        double theta = atan2(zy, zx);
        
        // Возведение в степень degree: z^n = r^n * (cos(n*theta) + i*sin(n*theta))
        double rn = pow(r, degree);
        if (iter == 0 && zx == 0 && zy == 0) { // Обработка начальной точки 0^n
             zx = cx;
             zy = cy;
        } else {
             zx = rn * cos(degree * theta) + cx;
             zy = rn * sin(degree * theta) + cy;
        }
        
        r2 = zx * zx + zy * zy;
        iter++;
    }

    Color color = get_spectral_color (iter, max_iter, r2, degree, R, G, B);

    return (0xFF000000) | (color.r << 16) | (color.g << 8) | color.b;
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

void ensure_directory(const char *dir) {
    struct stat st = {0};
    // Если папки нет, создаем её
    if (stat(dir, &st) == -1) {
        if (make_dir(dir) == 0) {
            printf("\nПапка '%s' создана.\n", dir);
        }
    }
}

void save_png_with_dpi(double zoom, double ca, double cb, int iter, double R, double G, double B) {

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
    // Директива OpenMP: разделяет итерации цикла по ядрам
    #pragma omp parallel for schedule(dynamic)
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            img[y * w + x] = compute_pixel(x, y, w, h, zoom, ca, cb, iter, R, G, B, degree);
        }
    }

    char basename[256];
    char final_path[512];

    const char *folder = "sdl_mouse";
    char *name = "mandelbrot_";
    char *ext = "png";
    snprintf(basename, sizeof(basename), "%s%dx%d_%ddpi", name, w, h, dpi);
    printf("basename = %s", basename);

    ensure_directory(folder);

    // Формируем путь внутри папки
    snprintf(final_path, sizeof(final_path), "%s/%s.%s", folder, basename, ext);

    // Проверяем на дубликаты
    int counter = 1;
    while (access(final_path, F_OK) == 0) {
        snprintf(final_path, sizeof(final_path), "%s/%s_%05d.%s", folder, basename, counter, ext);
        counter++;
    }

    // Записываем файл
    if (stbi_write_png(final_path, w, h, 4, img, w * 4)) {
        printf("\nУспешно сохранено: %s\n", final_path);
    }
    else {
        printf("\nОшибка при сохранении!\n");
    }

    free(img);
}

// Обычный рендер для окна
void render(SDL_Renderer *ren, SDL_Texture *tex, uint32_t *pix, ...) {

    va_list args;
    va_start(args, pix);
    RenderParam p;
    while ((p = va_arg(args, RenderParam)) != RENDER_END) {
        if (p == RENDER_ZOOM)
            zoom = va_arg(args, double);
        else if (p == RENDER_CA)
            ca = va_arg(args, double);
        else if (p == RENDER_CB)
            cb = va_arg(args, double);
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
        else if (p == RENDER_DEGREE)
            degree = va_arg(args, double);
    }
    va_end(args);

    // Директива OpenMP: разделяет итерации цикла по ядрам
    #pragma omp parallel for schedule(dynamic)
    for (int y = 0; y < H; y++)
        for (int x = 0; x < W; x++)
            pix[y * W + x] = compute_pixel(x, y, W, H, zoom, ca, cb, max_iter, R, G, B, degree);

    SDL_UpdateTexture(tex, NULL, pix, W * 4);
    SDL_RenderCopy(ren, tex, NULL, NULL);
    SDL_RenderPresent(ren);
}
