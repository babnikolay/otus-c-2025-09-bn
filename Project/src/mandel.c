#include "mandel.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

double zoom = 2.5, offsetX = 0.0, offsetY = 0.0;
int max_iter = 100, W = 800, H = 800;
double R = 255.0, G = 150.0, B = 0.0;
double degree = 2.0, freq = 0.1;

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
    printf("frequency   - Частота. [0.1]. Определяет, как быстро меняется цвет при переходе от одной итерации к другой.\n");
    printf("              !!! Не должна быть равна 0.\n");
    printf("              Низкая частота (например, 0.1): Цвет меняется медленно. Переходы будут широкими, плавными, \"растянутыми\".\n");
    printf("              Высокая частота (например, 1.0 или 2.0): Цвет меняется очень быстро.\n");
    printf("              Фрактал покроется множеством узких контрастных колец (эффект \"зебры\").\n");
    printf("MAX_ITER    - Максимальное количество итераций, за которое точка выйдет за границы множества. [1000]\n");
    printf("R           - Красный для палитры. [255.0]\n");
    printf("G           - Зелёный для палитры. [150.0]\n");
    printf("B           - Синий для палитры. [0.0]\n");
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

Color get_spectral_color (int iter, int max_iter, double zx, double zy, double degree,
                            double freq, double R, double G, double B) {
    
    if (iter >= max_iter) return (Color){0, 0, 0}; // Самое множество — черное

    double mu;
    // Защита от d=1 и d=0
    if (fabs(degree - 1.0) < 0.001 || degree < 0.001) {
        mu = (double)iter; 
    } else {
        double modul = sqrt(zx * zx + zy * zy);
        // Защита log(log) — значение внутри должно быть > 1
        if (modul > 1.001) {
            /*
            Алгоритм сглаживания (нормализованное количество итераций): mu = n + 1 - log(log|z|) / log(2)
            Для степеней отличных от 2, формула mu немного меняется: log(log|z|)/log(degree)
            Формула mu: Вычисляет "дробную итерацию".
            Теперь iter — это не целое число, а вещественное, что позволяет цвету меняться непрерывно.
            Вместо log(log(sqrt(zx² + zy²))) можно использовать свойства логарифмов, чтобы убрать корень:
            sqrt(z) = z^0.5
            log(z^0.5) = 0.5 * log(z)
            log(0.5 * log(z)) = log(0.5) + log(log(z))
            */
            const double log_05 = log(0.5);
            const double log_modul = log(modul);
            if (degree == 2) {
                const double inv_log2 = 1.0 / log(2.0);
                mu = iter + 1 - ((log_05 + log(log_modul)) * inv_log2);
            }
            else {
                const double inv_log_degree = 1.0 / log(degree);
                mu = (double)iter + 1.0 - ((log(log_modul) + log_05) * inv_log_degree);
                // mu = (double)iter + 1.0 - log(log(modul)) / log(degree);
            }
        } else {
            mu = (double)iter;
        }
    }

    if (isnan(mu) || isinf(mu)) mu = (double)iter;  // Проверка на NaN и бесконечность

    Color c;

    /*
    Создаем цикличный цветовой градиент на основе синусов sin(frequency * t + phase) для плавных цветовых переходов
    Это классический способ создать бесконечную "радугу", где фазовый сдвиг (числа phase_r, phase_g, phase_b) определяет баланс цветов
    Формула: 0.5 + 0.5 * sin(f * t + phase) масштабирует синус в диапазон [0, 1]
    Подбирая коэффициенты (phase_r=9.0, phase_g=15.0, phase_b=8.5), можно менять палитру
    */
    if (degree == 2) {
        c.r = (unsigned char)(sin(freq * mu + R) * 127 + 128);
        c.g = (unsigned char)(sin(freq * mu + G) * 127 + 128);
        c.b = (unsigned char)(sin(freq * mu + B) * 127 + 128);
    }
    else {
        // c.r = (unsigned char)(255 * (0.5 + 0.5 * sin(freq * mu + R)));
        // c.g = (unsigned char)(255 * (0.5 + 0.5 * sin(freq * mu + G)));
        // c.b = (unsigned char)(255 * (0.5 + 0.5 * sin(freq * mu + B)));

        // Более «ядовитые» и четкие цвета
        c.r = (unsigned char)(pow(0.5 * cos(freq * mu + R) + 0.5, 2.0) * 255);
        c.g = (unsigned char)(pow(0.5 * cos(freq * mu + G) + 0.5, 2.0) * 255);
        c.b = (unsigned char)(pow(0.5 * cos(freq * mu + B) + 0.5, 2.0) * 255);
    }

    return c;
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
uint32_t compute_pixel(int x, int y, int W, int H, double zoom, double freq, double offsetX, double offsetY, 
                          int max_iter, double R, double G, double B, double degree) {

    double radius = 4.0;
    int iter = 1;
    // Внутри цикла по x и y
    if (degree == 2.0) iter = 0;
    if (degree < 1.0) zoom = 10.0;
    if (degree > 10) radius = 16.0;    // Радиус выхода 16 - лучше для высоких степеней
    // Центрирование и масштабирование
    // double cx = offsetX + (x / (double)W - 0.5) * zoom;
    // double cy = offsetY + (y / (double)H - 0.5) * zoom;
    double cx = offsetX + (x - (double)W / 2.0) * (zoom / (double)W);
    double cy = offsetY + (y - (double)H / 2.0) * (zoom / (double)H);

    double zx = cx;
    double zy = cy;

    // --- УНИВЕРСАЛЬНЫЙ ЦИКЛ ДЛЯ ЛЮБОЙ СТЕПЕНИ ---
    while (zx*zx + zy*zy <= radius && iter < max_iter) {

        if (degree == 2) {
            double next_zx = zx * zx - zy * zy + cx;
            zy = 2 * zx * zy + cy;
            zx = next_zx;
        }
        else {
            // Переход в полярные координаты
            double r = sqrt(zx*zx + zy*zy);
            double theta = atan2(zy, zx);
            
            /*
            Формула: z = z^n + c
            𝐱_𝐧𝐞𝐱𝐭=𝐫^𝐧⋅𝐜𝐨𝐬⁡(𝐧⋅𝛉)+𝐱_𝐢𝐧𝐢𝐭
            𝐲_𝐧𝐞𝐱𝐭=𝐫^𝐧⋅𝐬𝐢𝐧(𝐧⋅𝛉)+𝐲_𝐢𝐧𝐢𝐭
            */
            double r_pow = pow(r, degree);
            double pow_theta = degree * theta;
            
            zx = r_pow * cos(pow_theta) + cx;
            zy = r_pow * sin(pow_theta) + cy;
        }

        iter++;
    }
            
    Color color = get_spectral_color (iter, max_iter, zx, zy, degree, freq, R, G, B);

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

void save_png_with_dpi(double zoom, double offsetX, double offsetY, int max_iter, double R, double G, double B) {

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
            img[y * w + x] = compute_pixel(x, y, w, h, zoom, freq, offsetX, offsetY, max_iter, R, G, B, degree);
        }
    }

    char basename[256];
    char final_path[512];

    const char *folder = "Images";
    char *name = "mandelbrot";
    char *ext = "png";
    snprintf(basename, sizeof(basename), "%s_%dx%d_%ddpi_%.3fdegree_%.3ffreq", name, w, h, dpi, degree, freq);
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
        else if (p == RENDER_FREQUENCY)
            freq = va_arg(args, double);
        else if (p == RENDER_OFFSETX)
            offsetX = va_arg(args, double);
        else if (p == RENDER_OFFSETY)
            offsetY = va_arg(args, double);
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
            pix[y * W + x] = compute_pixel(x, y, W, H, zoom, freq, offsetX, offsetY, max_iter, R, G, B, degree);

    SDL_UpdateTexture(tex, NULL, pix, W * 4);
    SDL_RenderCopy(ren, tex, NULL, NULL);
    SDL_RenderPresent(ren);
}
