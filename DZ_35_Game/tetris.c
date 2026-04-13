#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define COLS 10
#define ROWS 20
#define TILE_SIZE 30
#define PANEL_WIDTH 150
#define MAX_NAME 32

typedef enum { STATE_MENU, STATE_PLAYING, STATE_GAMEOVER } GameState;

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
char player_name[MAX_NAME] = "Player";
GameState state = STATE_MENU;

void save_score(void) {
    FILE *f = fopen("scores.txt", "a");
    if (f) { fprintf(f, "%s %d\n", player_name, score); fclose(f); }
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

int main(int argc, char *argv[]) {
    (void)argc; (void)argv;
    srand((unsigned int)time(NULL));
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *win = SDL_CreateWindow("C11 Tetris + Next Piece", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
                                      COLS*TILE_SIZE + PANEL_WIDTH, ROWS*TILE_SIZE, 0);
    SDL_Renderer *ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);

    Piece cur = {COLS/2-2, 0, rand()%7, 0};
    int next_type = rand()%7;
    uint32_t last_fall = SDL_GetTicks();
    bool quit = false;
    SDL_StartTextInput();

    while (!quit) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) quit = true;
            if (state == STATE_MENU) {
                if (e.type == SDL_TEXTINPUT) { if (strlen(player_name) < MAX_NAME - 1) strcat(player_name, e.text.text); }
                if (e.type == SDL_KEYDOWN) {
                    if (e.key.keysym.sym == SDLK_BACKSPACE && strlen(player_name) > 0) player_name[strlen(player_name)-1] = '\0';
                    if (e.key.keysym.sym == SDLK_RETURN) { state = STATE_PLAYING; SDL_StopTextInput(); }
                }
            } else if (state == STATE_PLAYING) {
                if (e.type == SDL_KEYDOWN) {
                    if (e.key.keysym.sym == SDLK_UP && !check_col(cur, 0, 0, 1)) cur.rot = (cur.rot+1)%4;
                    if (e.key.keysym.sym == SDLK_LEFT && !check_col(cur, -1, 0, 0)) cur.x--;
                    if (e.key.keysym.sym == SDLK_RIGHT && !check_col(cur, 1, 0, 0)) cur.x++;
                    if (e.key.keysym.sym == SDLK_DOWN && !check_col(cur, 0, 1, 0)) cur.y++;
                }
            }
        }

        if (state == STATE_PLAYING && SDL_GetTicks() - last_fall > 500) {
            if (!check_col(cur, 0, 1, 0)) cur.y++;
            else {
                for(int i=0; i<4; i++) for(int j=0; j<4; j++)
                    if(SHAPES[cur.type][cur.rot][i][j] && cur.y+i >= 0) grid[cur.y+i][cur.x+j] = cur.type + 1;
                clear_lines();
                cur = (Piece){COLS/2-2, 0, next_type, 0};
                next_type = rand()%7;
                if (check_col(cur, 0, 0, 0)) { state = STATE_GAMEOVER; save_score(); }
            }
            last_fall = SDL_GetTicks();
        }

        SDL_SetRenderDrawColor(ren, 15, 15, 25, 255); SDL_RenderClear(ren);

        if (state == STATE_MENU) {
            SDL_SetRenderDrawColor(ren, 0, 255, 100, 255);
            SDL_Rect r = {50, 250, 200, 50}; SDL_RenderFillRect(ren, &r);
        } else {
            // Игровое поле
            for(int i=0; i<ROWS; i++) for(int j=0; j<COLS; j++) if(grid[i][j]) {
                SDL_SetRenderDrawColor(ren, 70, 70, 150, 255);
                SDL_Rect r = {j*TILE_SIZE, i*TILE_SIZE, TILE_SIZE-1, TILE_SIZE-1}; SDL_RenderFillRect(ren, &r);
            }
            // Текущая фигура
            for(int i=0; i<4; i++) for(int j=0; j<4; j++) if(SHAPES[cur.type][cur.rot][i][j]) {
                SDL_SetRenderDrawColor(ren, 200, 50, 50, 255);
                SDL_Rect r = {(cur.x+j)*TILE_SIZE, (cur.y+i)*TILE_SIZE, TILE_SIZE-1, TILE_SIZE-1}; SDL_RenderFillRect(ren, &r);
            }
            // Панель "Следующая фигура"
            SDL_SetRenderDrawColor(ren, 40, 40, 60, 255);
            SDL_Rect panel = {COLS*TILE_SIZE, 0, PANEL_WIDTH, ROWS*TILE_SIZE}; SDL_RenderFillRect(ren, &panel);
            for(int i=0; i<4; i++) for(int j=0; j<4; j++) if(SHAPES[next_type][0][i][j]) {
                SDL_SetRenderDrawColor(ren, 100, 200, 100, 255);
                SDL_Rect r = {COLS*TILE_SIZE + 40 + j*20, 50 + i*20, 19, 19}; SDL_RenderFillRect(ren, &r);
            }
        }
        SDL_RenderPresent(ren);
        SDL_Delay(16);
    }
    SDL_DestroyRenderer(ren); SDL_DestroyWindow(win); SDL_Quit();
    return 0;
}
