all: mandel mandel_color mandel_smooth  mandel_anim mandel_sdl mandel_sdl_mouse

mandel: mandel.c
	gcc -O3 mandel.c -o mandel -Wall -Wextra -Wpedantic

mandel_color: mandel_color.c
	gcc -O3 mandel_color.c -o mandel_color -Wall -Wextra -Wpedantic

mandel_smooth: mandel_smooth.c
	gcc -O3 mandel_smooth.c -o mandel_smooth -Wall -Wextra -Wpedantic -lm

mandel_anim: mandel_anim.c
	gcc -O3 mandel_anim.c -o mandel_anim -Wall -Wextra -Wpedantic -lm

mandel_sdl: mandel_sdl.c
	gcc -O3 mandel_sdl.c -o mandel_sdl -Wall -Wextra -Wpedantic -lSDL2 -lm -fopenmp

mandel_sdl_mouse: mandel_sdl_mouse.c
	gcc -O3 mandel_sdl_mouse.c -o mandel_sdl_mouse -Wall -Wextra -Wpedantic -lSDL2 -lm -fopenmp

clean:
	rm -rf *.o *.ppm frames mandel mandel_color mandel_smooth mandel_anim mandel_sdl mandel_sdl_mouse
