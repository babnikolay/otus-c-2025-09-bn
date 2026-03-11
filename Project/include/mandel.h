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

typedef enum {
    RENDER_END = 0,
    RENDER_ZOOM,
    RENDER_CX,
    RENDER_CY,
    RENDER_ITER,
    RENDER_WIDTH,
    RENDER_HEIGHT,
    RENDER_R,
    RENDER_G,
    RENDER_B
} RenderParam;

void help();

int is_valid_number(const char *str);

void get_input(const char *prompt, void *variable, const char *type, const char *default_val);

uint32_t compute_pixel(int x, int y, int W, int H, double zoom, double cx, double cy, int iter_count, double R, double G, double B);

void clear_stdin();

int get_int_default(const char *prompt, int default_val);

void ensure_directory(const char *dir);

void save_png_with_dpi(double zoom, double cx, double cy, int iter, double R, double G, double B);

void render(SDL_Renderer *ren, SDL_Texture *tex, uint32_t *pix, ...);

#define RENDER_SAFE(ren, tex, pix, ...) render(ren, tex, pix, ##__VA_ARGS__, RENDER_END)

#endif
