# Makefile
CC = gcc
SRC ?=
OUT ?=
CFLAGS = -Wall -Wextra -Wpedantic -std=c11

# ifdef CUBE
# 	SRC = cube.c
# 	OUT = cube
# 	LDLIBS = -lGL -lGLU -lglut
# else
# 	SRC = texture_cube.c
# 	OUT = texture_cube
# 	LDLIBS = -lGL -lGLU -lglut -lGLEW -lglfw -lSOIL -lm
# endif

ifeq ($(cube),1)
	SRC = cube.c
	OUT = cube
	LDLIBS = -lGL -lGLU -lglut
else ifeq ($(cube),0)
	SRC = texture_cube.c
	OUT = texture_cube
	LDLIBS = -lGL -lGLU -lglut -lGLEW -lglfw -lSOIL -lm
else
	LDLIBS = -lGL -lGLU -lglut
endif


OBJ = $(SRC:.c=.o)

.PHONY: all clean

all: $(OUT)

$(OUT): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
# 	rm -f $(OBJ) $(OUT)
	rm -f *.o
