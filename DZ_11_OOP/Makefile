CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -std=c11 `pkg-config --cflags gtk+-3.0`
LIBS = `pkg-config --libs gtk+-3.0`

TARGET = app_gtk
SRC = app_gtk.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) $(LIBS)

clean:
	rm -f $(TARGET)

.PHONY: all clean
