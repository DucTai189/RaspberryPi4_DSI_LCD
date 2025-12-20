CC = gcc
CFLAGS = -Wall -g `sdl2-config --cflags`
LIBS =  -lgpiod `sdl2-config --libs` -lSDL2_image -lm

SRCS = main.c GPIO_Init.c Touchscreen.c
OBJS = $(SRCS:.c=.o)
TARGET = DSI_LCD

ifdef DEBUG
    CFLAGS += -g -DDEBUG
else
    CFLAGS += -O2
endif

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS)	-o $@ $^ $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS)	-c	$<

clean:
	rm	-f $(OBJS) $(TARGET)
.PHONY: all clean