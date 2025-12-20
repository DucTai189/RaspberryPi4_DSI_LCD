#ifndef TOUCHSCREEN_H
#define TOUCHSCREEN_H

#define WIDTH_LCD 800
#define HEITH_LCD 480

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#define FB_DEVICE "/dev/fb0"

#define TOUCH_DEVICE "/dev/input/event0"
#define GPIO_CHIP "/dev/gpiochip0"

typedef struct {
    unsigned int width;
    unsigned int height;
    unsigned int bpp;
    unsigned int size;
    unsigned char *buffer;
    int fd;
} Framebuffer;

typedef struct {
    unsigned char *pixels;
    int width;
    int height;
    int channels;
} Image;

typedef struct 
{
    uint16_t x; // position 
    uint16_t y; // position 
    uint16_t width; // width of objective  
    uint16_t height ; // height  of objective  
} Rect_st;

int init_framebuffer(Framebuffer *fb) ;
int load_image(const char *filename, Image *img) ;
void clear_screen(Framebuffer *fb, unsigned int color) ;
void draw_image(Framebuffer *fb, Image *img, Rect_st *rect) ;

#endif