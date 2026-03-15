#ifndef MANDEL_H  // Защита от повторного включения
#define MANDEL_H

#include <SDL2/SDL.h>
#include <omp.h>
#include <stdarg.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h> // Для access()
#include <sys/stat.h>

// В C по умолчанию нет M_PI, определим на всякий случай
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define make_dir(path) mkdir(path, 0777)

#define PALETTE_SIZE 65535  // Размер таблицы для плавности цветов

typedef struct {
    unsigned char r, g, b;
} Color;

typedef enum {
    RENDER_END = 0,
    RENDER_ZOOM,
    RENDER_CA,
    RENDER_CB,
    RENDER_ITER,
    RENDER_WIDTH,
    RENDER_HEIGHT,
    RENDER_R,
    RENDER_G,
    RENDER_B,
    RENDER_DEGREE
} RenderParam;

void help();

Color get_spectral_color (int iter, int max_iter, double modul, double degree, double R, double G, double B);

// void generate_palette();

// Color lin_interpol_color (Color c1, Color c2, double t);

// Color get_mandel_color (int n, double z_modul, int max_iter);

int is_valid_number(const char *str);

void get_input(const char *prompt, void *variable, const char *type, const char *default_val);

uint32_t compute_pixel(int x, int y, int W, int H, double zoom, double ca, double cb, 
                        int max_iter, double R, double G, double B, double degree);

// uint32_t compute_pixel(int x, int y, int W, int H, double zoom, double cx, double cy, int max_iter, double degree);

void clear_stdin();

int get_int_default(const char *prompt, int default_val);

void ensure_directory(const char *dir);

void save_png_with_dpi(double zoom, double cx, double cy, int iter, double R, double G, double B);

void render(SDL_Renderer *ren, SDL_Texture *tex, uint32_t *pix, ...);

#define RENDER_SAFE(ren, tex, pix, ...) render(ren, tex, pix, ##__VA_ARGS__, RENDER_END)

#endif
