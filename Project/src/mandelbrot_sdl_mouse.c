#include <stdio.h>
#include <time.h>
#include <ctype.h>
#include <string.h>
#include "mandel.h"

#define BOLD_BLUE "\033[94m"
#define RESET      "\033[0m"

int main() {
    
    // 1. Фиксируем время начала
    time_t start_time = time(NULL);
    printf("Начало: %s\n", ctime(&start_time));
    fflush(stdout);

    extern double zoom, offsetX, offsetY;
    extern int W, H, max_iter;
    extern double R, G, B;
    extern double degree, freq;

    char input[10];

    while (1) {
        printf("Сейчас применены параметры по умолчанию.\n");
        printf("Для подробности нажмите h.\n");
        printf("Хотите изменить параметры? (yY/"BOLD_BLUE"[nN]"RESET"/hH): ");
        if (fgets(input, sizeof(input), stdin) == NULL) break;
        input[strcspn(input, "\n")] = 0;

        // Сравнивает, игнорируя регистр (HELP == help == HeLp)
        if (strcasecmp(input, "h") == 0) {
            help();

            // 2. Фиксируем время окончания
            time_t end_time = time(NULL);
            printf("\nОкончание: %s\n", ctime(&end_time));

            // 3. Вычисляем разницу (длительность)
            double diff = difftime(end_time, start_time);
            printf("Программа работала %.0f сек.\n", diff);

            return 0;
        } else if (strcasecmp(input, "y") == 0) {
            // Изменение начальных значений
            get_input("Ширина окна - width: ", &W, "int", "800");
            get_input("Высота окна - height: ", &H, "int", "800");
            get_input("Максимальное количество итераций - MAX_ITER: ", &max_iter, "int", "100");
            get_input("Красный для палитры: ", &R, "double", "255.0");
            get_input("Зелёный для палитры: ", &G, "double", "150.0");
            get_input("Синий для палитры: ", &B, "double", "0.0");
            get_input("Степень числа Z в формуле Мандельброта: ", &degree, "double", "2.0");
            get_input("Частота изменения цвета: ", &freq, "double", "0.1");

            printf("\nДанные приняты:\n");
            printf("W: %d, H: %d, MAX_ITER: %d, frequency: %.3f\n", W, H, max_iter, freq);
            printf("R: %.3f, G: %.3f, B: %.3f, degree: %.3f\n", R, G, B, degree);

            break;
        } else if (input[0] == '\0' || strcasecmp(input, "n") == 0) {
            // Ввод параметров по умолчанию
            printf("Применены параметры по умолчанию!!!\n");
            printf("\nДанные по умолчанию:\n");
            printf("W: %d, H: %d, MAX_ITER: %d, frequency: %.3f\n", W, H, max_iter, freq);
            printf("R: %.3f, G: %.3f, B: %.3f, degree: %.3f\n", R, G, B, degree);
            break;
        } else {
            printf("Неизвестная команда. Повторите ввод.\n");
        }
    }

    if (degree == 2.0) {
        offsetX = -0.75;
    }

    // Задание начальных параметров для окна
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *win = SDL_CreateWindow("Mandelbrot [S - Save]", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, W, H, 0);
    SDL_Renderer *ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
    SDL_Texture *tex = SDL_CreateTexture(ren, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, W, H);
    uint32_t *pixels = malloc(W * H * sizeof(uint32_t));

    int running = 1;
    SDL_Event event;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT)
                running = 0;
            
            // Обработка событий клавиатуры
            if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                    case SDLK_s: // Нажать S для сохранения
                        save_png_with_dpi(zoom, offsetX, offsetY, max_iter, R, G, B);
                        printf("Re: %.18f | Im: %.18f | Zoom: %.2e", offsetX, offsetY, zoom);
                        fflush(stdout);
                        break;
                    case SDLK_ESCAPE: running = 0; break;
                    case SDLK_UP: offsetY -= 0.1 * zoom; break;
                    case SDLK_DOWN: offsetY += 0.1 * zoom; break;
                    case SDLK_LEFT: offsetX -= 0.1 * zoom; break;
                    case SDLK_RIGHT: offsetX += 0.1 * zoom; break;
                    case SDLK_EQUALS: zoom *= 0.8; break;
                    case SDLK_MINUS: zoom *= 1.3; break;
                }
            }
            
            // Обработка событий мыши
            if (event.type == SDL_MOUSEBUTTONDOWN) {
                // double m_cx = offsetX + (event.button.x - (double)W / 2.5) * (zoom / (double)W);
                // double m_cy = offsetY + (event.button.y - (double)H / 2.5) * (zoom / (double)H);
                // double mouse_x_ratio = (event.button.x / (long double)W - 0.5);
                // double mouse_y_ratio = (event.button.y / (long double)H - 0.5);
                double mouse_x_ratio = (event.button.x - (double)W / 2.5);
                double mouse_y_ratio = (event.button.y - (double)H / 2.5);
                double zoom_x_ratio = (zoom / (double)W);
                double zoom_y_ratio = (zoom / (double)H);

                // Вычисляем новую центральную точку
                double m_cx = offsetX + mouse_x_ratio * zoom_x_ratio;
                double m_cy = offsetY + mouse_y_ratio * zoom_y_ratio;

                offsetX = m_cx;
                offsetY = m_cy;
                if (event.button.button == SDL_BUTTON_LEFT)
                    zoom *= 0.8;
                else
                    zoom *= 1.2;

                // Вывод координат точки в заголовке окна
                char title[256];
                snprintf(title, sizeof(title), "Mandelbrot [S - Save] | Re: %.18f | Im: %.18f | Zoom: %.2e", offsetX, offsetY, zoom);
                SDL_SetWindowTitle(win, title);
            }
        }
        
        RENDER_SAFE(ren, tex, pixels, 
                    RENDER_ZOOM, zoom,
                    RENDER_FREQUENCY, freq, 
                    RENDER_OFFSETX, offsetX, 
                    RENDER_OFFSETY, offsetY, 
                    RENDER_ITER, max_iter, 
                    RENDER_WIDTH, W, 
                    RENDER_HEIGHT, H, 
                    RENDER_R, R, 
                    RENDER_G, G, 
                    RENDER_B, B, 
                    RENDER_DEGREE, degree);

    }

    // Очистка параметров окна
    free(pixels);
    SDL_DestroyTexture(tex);
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();

    // 2. Фиксируем время окончания
    time_t end_time = time(NULL);
    printf("\nОкончание: %s\n", ctime(&end_time));

    // 3. Вычисляем разницу (длительность)
    double diff = difftime(end_time, start_time);
    printf("Программа работала %.0f сек.\n", diff);

    return 0;
}
