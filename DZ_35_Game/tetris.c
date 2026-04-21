#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define COLS 10
#define ROWS 20
#define TILE_SIZE 30
#define PANEL_WIDTH 200
#define MAX_NAME 32

typedef enum { STATE_MENU, STATE_PLAYING, STATE_GAMEOVER } GameState;

const SDL_Color PIECE_COLORS[] = {
    {0, 255, 255, 255}, {255, 255, 0, 255}, {128, 0, 128, 255},
    {0, 255, 0, 255},   {255, 0, 0, 255},   {0, 0, 255, 255}, {255, 165, 0, 255}
};

const int SHAPES[7][4][4][4] = {
    {{{0,1,0,0},{0,1,0,0},{0,1,0,0},{0,1,0,0}}, {{0,0,0,0},{1,1,1,1},{0,0,0,0},{0,0,0,0}}, {{0,1,0,0},{0,1,0,0},{0,1,0,0},{0,1,0,0}}, {{0,0,0,0},{1,1,1,1},{0,0,0,0},{0,0,0,0}}}, // I
    {{{1,1,0,0},{1,1,0,0},{0,0,0,0},{0,0,0,0}}, {{1,1,0,0},{1,1,0,0},{0,0,0,0},{0,0,0,0}}, {{1,1,0,0},{1,1,0,0},{0,0,0,0},{0,0,0,0}}, {{1,1,0,0},{1,1,0,0},{0,0,0,0},{0,0,0,0}}}, // O
    {{{0,1,0,0},{1,1,1,0},{0,0,0,0},{0,0,0,0}}, {{0,1,0,0},{0,1,1,0},{0,1,0,0},{0,0,0,0}}, {{0,0,0,0},{1,1,1,0},{0,1,0,0},{0,0,0,0}}, {{0,1,0,0},{1,1,0,0},{0,1,0,0},{0,0,0,0}}}, // T
    {{{0,1,1,0},{1,1,0,0},{0,0,0,0},{0,0,0,0}}, {{0,1,0,0},{0,1,1,0},{0,0,1,0},{0,0,0,0}}, {{0,1,1,0},{1,1,0,0},{0,0,0,0},{0,0,0,0}}, {{0,1,0,0},{0,1,1,0},{0,0,1,0},{0,0,0,0}}}, // S
    {{{1,1,0,0},{0,1,1,0},{0,0,0,0},{0,0,0,0}}, {{0,0,1,0},{0,1,1,0},{0,1,0,0},{0,0,0,0}}, {{1,1,0,0},{0,1,1,0},{0,0,0,0},{0,0,0,0}}, {{0,0,1,0},{0,1,1,0},{0,1,0,0},{0,0,0,0}}}, // Z
    {{{1,0,0,0},{1,1,1,0},{0,0,0,0},{0,0,0,0}}, {{0,1,1,0},{0,1,0,0},{0,1,0,0},{0,0,0,0}}, {{0,0,0,0},{1,1,1,0},{0,0,1,0},{0,0,0,0}}, {{0,1,0,0},{0,1,0,0},{1,1,0,0},{0,0,0,0}}}, // J
    {{{0,0,1,0},{1,1,1,0},{0,0,0,0},{0,0,0,0}}, {{0,1,0,0},{0,1,0,0},{0,1,1,0},{0,0,0,0}}, {{0,0,0,0},{1,1,1,0},{1,0,0,0},{0,0,0,0}}, {{1,1,0,0},{0,1,0,0},{0,1,0,0},{0,0,0,0}}}  // L
};

typedef struct { int x, y, type, rot; } Piece;

int grid[ROWS][COLS] = {0};
int score = 0;
char player_name[MAX_NAME] = "";
GameState state = STATE_MENU;

void draw_text(SDL_Renderer *ren, TTF_Font *font, const char *text, int x, int y, SDL_Color color) {
    if (!text || text[0] == '\0') return;
    SDL_Surface *surf = TTF_RenderText_Blended(font, text, color);
    if (!surf) return;
    SDL_Texture *tex = SDL_CreateTextureFromSurface(ren, surf);
    SDL_Rect dst = {x, y, surf->w, surf->h};
    SDL_RenderCopy(ren, tex, NULL, &dst);
    SDL_FreeSurface(surf);
    SDL_DestroyTexture(tex);
}

void save_score(void) {
    const char *final_name = (strlen(player_name) > 0) ? player_name : "Player";
    FILE *f = fopen("scores.txt", "a");
    if (f) { fprintf(f, "%s %d\n", final_name, score); fclose(f); }
}

bool check_col(Piece p, int dx, int dy, int dr) {
    int nr = (p.rot + dr) % 4;
    for(int i=0; i<4; i++) for(int j=0; j<4; j++) {
        if(SHAPES[p.type][nr][i][j]) {
            int nx = p.x + j + dx, ny = p.y + i + dy;
            if(nx < 0 || nx >= COLS || ny >= ROWS || (ny >= 0 && grid[ny][nx])) return true;
        }
    }
    return false;
}

void clear_lines(void) {
    for(int i = ROWS-1; i >= 0; i--) {
        bool full = true;
        for(int j=0; j<COLS; j++) if(!grid[i][j]) full = false;
        if(full) {
            score += 100;
            for(int k=i; k>0; k--) memcpy(grid[k], grid[k-1], sizeof(int)*COLS);
            memset(grid[0], 0, sizeof(int)*COLS);
            i++;
        }
    }
}

void handle_input(SDL_Event *e, bool *quit, Piece *cur) {
    if (e->type == SDL_QUIT) *quit = true;

    if (state == STATE_MENU) {
        if (e->type == SDL_TEXTINPUT) {
            if (strlen(player_name) < MAX_NAME - 1) strcat(player_name, e->text.text);
        }
        if (e->type == SDL_KEYDOWN) {
            if (e->key.keysym.sym == SDLK_BACKSPACE && strlen(player_name) > 0) 
                player_name[strlen(player_name)-1] = '\0';
            if (e->key.keysym.sym == SDLK_RETURN) { 
                state = STATE_PLAYING; 
                SDL_StopTextInput(); 
            }
        }
    } else if (state == STATE_PLAYING) {
        if (e->type == SDL_KEYDOWN) {
            if (e->key.keysym.sym == SDLK_UP && !check_col(*cur, 0, 0, 1)) cur->rot = (cur->rot + 1) % 4;
            if (e->key.keysym.sym == SDLK_LEFT && !check_col(*cur, -1, 0, 0)) cur->x--;
            if (e->key.keysym.sym == SDLK_RIGHT && !check_col(*cur, 1, 0, 0)) cur->x++;
            if (e->key.keysym.sym == SDLK_DOWN && !check_col(*cur, 0, 1, 0)) cur->y++;
        }
    } else if (state == STATE_GAMEOVER) {
        if (e->type == SDL_KEYDOWN) { 
            state = STATE_MENU; score = 0; 
            memset(grid, 0, sizeof(grid)); 
            SDL_StartTextInput(); 
        }
    }
}

void update_game(Piece *cur, int *next_type, uint32_t *last_fall) {
    if (state != STATE_PLAYING || SDL_GetTicks() - *last_fall <= 500) return;

    if (!check_col(*cur, 0, 1, 0)) {
        cur->y++;
    } else {
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                if (SHAPES[cur->type][cur->rot][i][j] && cur->y + i >= 0)
                    grid[cur->y + i][cur->x + j] = cur->type + 1;
            }
        }
        clear_lines();
        *cur = (Piece){COLS / 2 - 2, 0, *next_type, 0};
        *next_type = rand() % 7;
        if (check_col(*cur, 0, 0, 0)) { 
            state = STATE_GAMEOVER; 
            save_score(); 
        }
    }
    *last_fall = SDL_GetTicks();
}

void render(SDL_Renderer *ren, TTF_Font *font, Piece cur, int next_type) {
    SDL_SetRenderDrawColor(ren, 10, 10, 20, 255);
    SDL_RenderClear(ren);
    SDL_Color white = {255, 255, 255, 255};

    if (state == STATE_MENU) {
        draw_text(ren, font, "ENTER NAME:", 50, 180, white);
        draw_text(ren, font, player_name, 60, 245, (SDL_Color){0, 255, 100, 255});
        SDL_SetRenderDrawColor(ren, 0, 255, 100, 255);
        SDL_Rect r = {50, 240, 350, 40}; SDL_RenderDrawRect(ren, &r);
        draw_text(ren, font, "Press ENTER to Start", 100, 320, white);
    } else {
        // 1. Отрисовка правой панели
        SDL_SetRenderDrawColor(ren, 25, 25, 35, 255);
        SDL_Rect info_panel = {COLS * TILE_SIZE, 0, PANEL_WIDTH, ROWS * TILE_SIZE};
        SDL_RenderFillRect(ren, &info_panel);

        // 2. Отрисовка сетки (Grid)
        SDL_SetRenderDrawColor(ren, 30, 30, 45, 255);
        for(int i=0; i<=COLS; i++) SDL_RenderDrawLine(ren, i*TILE_SIZE, 0, i*TILE_SIZE, ROWS*TILE_SIZE);
        for(int i=0; i<=ROWS; i++) SDL_RenderDrawLine(ren, 0, i*TILE_SIZE, COLS*TILE_SIZE, i*TILE_SIZE);

        // Разделительная линия
        SDL_SetRenderDrawColor(ren, 255, 255, 255, 255);
        SDL_Rect sep = {COLS * TILE_SIZE, 0, 2, ROWS * TILE_SIZE};
        SDL_RenderFillRect(ren, &sep);

        // 3. Отрисовка зафиксированных блоков в стакане
        for(int i=0; i<ROWS; i++) {
            for(int j=0; j<COLS; j++) {
                if(grid[i][j]) {
                    SDL_Color c = PIECE_COLORS[grid[i][j]-1];
                    SDL_SetRenderDrawColor(ren, c.r, c.g, c.b, 255);
                    SDL_Rect r = {j*TILE_SIZE + 1, i*TILE_SIZE + 1, TILE_SIZE-2, TILE_SIZE-2};
                    SDL_RenderFillRect(ren, &r);
                }
            }
        }

        // 4. Отрисовка текущей падающей фигуры
        SDL_Color cur_c = PIECE_COLORS[cur.type];
        SDL_SetRenderDrawColor(ren, cur_c.r, cur_c.g, cur_c.b, 255);
        for(int i=0; i<4; i++) {
            for(int j=0; j<4; j++) {
                if(SHAPES[cur.type][cur.rot][i][j]) {
                    SDL_Rect r = {(cur.x+j)*TILE_SIZE + 1, (cur.y+i)*TILE_SIZE + 1, TILE_SIZE-2, TILE_SIZE-2};
                    SDL_RenderFillRect(ren, &r);
                }
            }
        }

        // 5. Отрисовка следующей фигуры (Next)
        draw_text(ren, font, "NEXT:", COLS*TILE_SIZE + 30, 30, white);
        SDL_Color next_c = PIECE_COLORS[next_type];
        SDL_SetRenderDrawColor(ren, next_c.r, next_c.g, next_c.b, 255);
        for(int i=0; i<4; i++) {
            for(int j=0; j<4; j++) {
                if(SHAPES[next_type][0][i][j]) {
                    SDL_Rect r = {COLS*TILE_SIZE + 50 + j*25, 80 + i*25, 23, 23};
                    SDL_RenderFillRect(ren, &r);
                }
            }
        }

        // 6. Отрисовка счета
        char score_str[32]; 
        sprintf(score_str, "SCORE: %d", score);
        draw_text(ren, font, score_str, COLS*TILE_SIZE + 30, 220, white);

        if(state == STATE_GAMEOVER) {
            draw_text(ren, font, "GAME OVER", 50, 300, (SDL_Color){255, 0, 0, 255});
        }
    }
    SDL_RenderPresent(ren);
}


int main() {
    srand((unsigned int)time(NULL));
    if (SDL_Init(SDL_INIT_VIDEO) < 0 || TTF_Init() < 0) return 1;

    SDL_Window *win = SDL_CreateWindow("Tetris C11", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
                                      COLS*TILE_SIZE + PANEL_WIDTH, ROWS*TILE_SIZE, 0);
    SDL_Renderer *ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
    TTF_Font *font = TTF_OpenFont("font.ttf", 24); 

    Piece cur = {COLS/2-2, 0, rand()%7, 0};
    int next_type = rand()%7;
    uint32_t last_fall = SDL_GetTicks();
    bool quit = false;

    SDL_StartTextInput();

    while (!quit) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            handle_input(&e, &quit, &cur);
        }

        update_game(&cur, &next_type, &last_fall);
        render(ren, font, cur, next_type);
        
        SDL_Delay(16);
    }

    TTF_CloseFont(font);
    TTF_Quit(); SDL_DestroyRenderer(ren); SDL_DestroyWindow(win); SDL_Quit();
    return 0;
}
