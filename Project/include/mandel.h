#ifndef MANDEL_H  // Защита от повторного включения
#define MANDEL_H

#include <SDL2/SDL.h>
#include <omp.h>
#include <stdarg.h>
#include <math.h>
#include <stdint.h>

typedef enum
{
    RENDER_END = 0,
    RENDER_CX,
    RENDER_CY,
    RENDER_ITER,
    RENDER_WIDTH,
    RENDER_HEIGHT
} RenderParam;

void help();

uint32_t compute_pixel(int x, int y, int W, int H, double zoom, double cx, double cy, int iter_count);

void save_png_with_dpi(double zoom, double cx, double cy, int iter);

void render(SDL_Renderer *ren, SDL_Texture *tex, uint32_t *pix, double zoom, ...);

#define RENDER_SAFE(ren, tex, pix, zoom, ...) render(ren, tex, pix, zoom, ##__VA_ARGS__, RENDER_END)

#endif
