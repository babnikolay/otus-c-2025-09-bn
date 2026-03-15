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

    extern double zoom, ca, cb;
    extern int W, H, max_iter;
    extern double R, G, B;
    extern double degree;

    int running = 1;
    SDL_Event event;

    // generate_palette();

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
            get_input("Масштабирование - zoom: ", &zoom, "double", "2.5");
            get_input("Максимальное количество итераций - MAX_ITER: ", &max_iter, "int", "1000");
            get_input("Красный для палитры: ", &R, "double", "0.0");
            get_input("Зелёный для палитры: ", &G, "double", "2.094");
            get_input("Синий для палитры: ", &B, "double", "4.188");
            get_input("Степень Z в формуле Мандельброта: ", &degree, "double", "2.0");

            printf("\nДанные приняты:\n");
            printf("W: %d, H: %d, Zoom: %.2f, MAX_ITER: %d\n", W, H, zoom, max_iter);
            printf("R: %.3f, G: %.3f, B: %.3f, degree: %.3f\n", R, G, B, degree);

            break;
        } else if (input[0] == '\0' || strcasecmp(input, "n") == 0) {
            // Ввод параметров по умолчанию
            printf("Применены параметры по умолчанию!!!\n");
            printf("\nДанные по умолчанию:\n");
            printf("W: %d, H: %d, Zoom: %.2f, MAX_ITER: %d\n", W, H, zoom, max_iter);
            printf("R: %.3f, G: %.3f, B: %.3f, degree: %.3f\n", R, G, B, degree);
            break;
        } else {
            printf("Неизвестная команда. Повторите ввод.\n");
        }
    }

    if (degree >= 3) {
        ca = 0.0;
        cb = 0.0;
    }

    // Задание начальных параметров для окна
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *win = SDL_CreateWindow("Mandelbrot [S - Save]", 100, 100, W, H, 0);
    SDL_Renderer *ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
    SDL_Texture *tex = SDL_CreateTexture(ren, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, W, H);
    uint32_t *pixels = malloc(W * H * 4);

    while (running) {
        /*
        Базовое количество итераций (например, 100) + добавка за зум
        Чем меньше zoom, тем больше итераций. Формула: 150 + 40 * |log10(zoom)|
        */
        int cur_iter = 150 + (int)(40.0 * fabs(log10(zoom)));
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT)
                running = 0;
            
            // Обработка событий клавиатуры
            if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                    case SDLK_s: // Нажать S для сохранения
                        save_png_with_dpi(zoom, ca, cb, cur_iter, R, G, B);
                        break;
                    case SDLK_ESCAPE: running = 0; break;
                    case SDLK_UP: cb -= 0.1 * zoom; break;
                    case SDLK_DOWN: cb += 0.1 * zoom; break;
                    case SDLK_LEFT: ca -= 0.1 * zoom; break;
                    case SDLK_RIGHT: ca += 0.1 * zoom; break;
                    case SDLK_EQUALS: zoom *= 0.8; break;
                    case SDLK_MINUS: zoom *= 1.2; break;
                }
            }
            
            // Обработка событий мыши
            if (event.type == SDL_MOUSEBUTTONDOWN) {
                double m_ca = ca + (event.button.x - W / 2.5) * (zoom / W);
                double m_cb = cb + (event.button.y - H / 2.5) * (zoom / H);
                ca = m_ca;
                cb = m_cb;
                if (event.button.button == SDL_BUTTON_LEFT)
                    zoom *= 0.9;
                else
                    zoom /= 0.9;
            }
        }
        
        RENDER_SAFE(ren, tex, pixels, 
                    RENDER_ZOOM, zoom, 
                    RENDER_CA, ca, 
                    RENDER_CB, cb, 
                    RENDER_ITER, cur_iter, 
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
