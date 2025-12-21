#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "Touchscreen.h"
#include <linux/fb.h>
#include <linux/input.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <fcntl.h>


int init_framebuffer(Framebuffer *fb) 
{
    struct fb_var_screeninfo vinfo;
    struct fb_fix_screeninfo finfo;
    
    fb->fd = open(FB_DEVICE, O_RDWR);
    if (fb->fd < 0) {
        perror("Cannot open framebuffer");
        return -1;
    }
    
    if (ioctl(fb->fd, FBIOGET_FSCREENINFO, &finfo) || 
        ioctl(fb->fd, FBIOGET_VSCREENINFO, &vinfo)) {
        perror("Failed to get screen info");
        close(fb->fd);
        return -1;
    }
    
    fb->width = vinfo.xres;
    fb->height = vinfo.yres;
    fb->bpp = vinfo.bits_per_pixel;
    fb->size = finfo.smem_len;
    
    printf("Display: %dx%d, %dbpp\n", fb->width, fb->height, fb->bpp);
    
    fb->buffer = (unsigned char*)mmap(0, fb->size, PROT_READ | PROT_WRITE, 
                                      MAP_SHARED, fb->fd, 0);
    if (fb->buffer == MAP_FAILED) {
        perror("Failed to mmap framebuffer");
        close(fb->fd);
        return -1;
    }
    
    return 0;
}

int load_image(const char *filename, Image *img) 
{
    img->pixels = stbi_load(filename, &img->width, &img->height, &img->channels, 4);
    if (!img->pixels) 
    {
        fprintf(stderr, "Failed to load %s: %s\n", filename, stbi_failure_reason());
        return -1;
    }
   // printf("Loaded image: %dx%d, %d channels\n", img->width, img->height, img->channels);
    return 0;
}

void clear_screen(Framebuffer *fb, unsigned int color) 
{
    for (unsigned int i = 0; i < fb->size; i += 4) 
    {
        *((unsigned int*)(fb->buffer + i)) = color;
    }
}

void draw_image(Framebuffer *fb, Image *img, Rect_st *rect) 
{
    for (int dy = 0; dy < rect->height; dy++) 
    {
        for (int dx = 0; dx < rect->width; dx++) 
        {
            int sx = (dx * img->width) / rect->width;
            int sy = (dy * img->height) / rect->height;
            
            int src_idx = (sy * img->width + sx) * 4;
            int dst_x = rect->x + dx;
            int dst_y = rect->y + dy;
            
            if (dst_x >= 0 && dst_x < (int)fb->width && 
                dst_y >= 0 && dst_y < (int)fb->height) {
                
                int dst_idx = (dst_y * fb->width + dst_x) * (fb->bpp / 8);
                
                unsigned char r = img->pixels[src_idx + 0];
                unsigned char g = img->pixels[src_idx + 1];
                unsigned char b = img->pixels[src_idx + 2];
                unsigned char a = img->pixels[src_idx + 3];
                
                if (a == 255) 
                {
                    unsigned int color = (r << 16) | (g << 8) | b;
                    *((unsigned int*)(fb->buffer + dst_idx)) = color;
                } 
                else if (a > 0) 
                {
                    r = (r * a) / 255;
                    g = (g * a) / 255;
                    b = (b * a) / 255;
                    unsigned int color = (r << 16) | (g << 8) | b;
                    *((unsigned int*)(fb->buffer + dst_idx)) = color;
                }
                else
                {
                    /* Do nothing*/
                }
            }
        }
    }
}


