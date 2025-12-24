#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "Touchscreen.h"
#include <linux/fb.h>
#include <linux/input.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include "TV_FONT.h"

int init_framebuffer(Framebuffer *fb) 
{
    struct fb_var_screeninfo vinfo;
    struct fb_fix_screeninfo finfo;
    
    fb->fd = open(FB_DEVICE, O_RDWR);
    if (fb->fd < 0) 
    {
        perror("Cannot open framebuffer");
        return -1;
    }
    
    if (ioctl(fb->fd, FBIOGET_FSCREENINFO, &finfo) || 
        ioctl(fb->fd, FBIOGET_VSCREENINFO, &vinfo)) 
    {
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
    if (fb->buffer == MAP_FAILED) 
    {
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
    uint16_t  dx, dy = 0;
    uint32_t  sx,sy = 0;
    uint32_t  src_idx = 0;
    uint16_t  dst_x, dst_y = 0;
    uint32_t  dst_idx = 0; 
    uint8_t   red, green, blue, alpha = 0;   
    uint32_t  color = 0;

    // Scan the row
    for (dy = 0; dy < rect->height; dy++) 
    {
        // Scan the column
        for (dx = 0; dx < rect->width; dx++) 
        {
            //Scaling the row and column
            sx = (dx * img->width) / rect->width;
            sy = (dy * img->height) / rect->height;
            
            // Calculate Source Pixel Index
            // Converts 2D coordinates (sx, sy) to 1D array index
            // Multiple 4 because each pixel has 4 bytes: R, G, B, A
            src_idx = (sy * img->width + sx) * 4;

            // Converts relative position (dx, dy) to absolute screen coordinates.
            dst_x = rect->x + dx;
            dst_y = rect->y + dy;
            
            if (dst_x >= 0 && dst_x < (uint16_t)fb->width && 
                dst_y >= 0 && dst_y < (uint16_t)fb->height) 
            {
                
                dst_idx = (dst_y * fb->width + dst_x) * (fb->bpp / 8);
                
                red   = img->pixels[src_idx + 0];
                green = img->pixels[src_idx + 1];
                blue  = img->pixels[src_idx + 2];
                alpha = img->pixels[src_idx + 3];
                
                if (alpha == 255) 
                {
                   color = (red << 16) | (green << 8) | blue;
                   *((uint32_t*)(fb->buffer + dst_idx)) = color;
                } 
                else if (alpha > 0) 
                {
                    red = (red * alpha) / 255;
                    green = (green * alpha) / 255;
                    blue = (blue * alpha) / 255;
                    color = (red << 16) | (green << 8) | blue;
                    *((uint32_t*)(fb->buffer + dst_idx)) = color;
                }
                else
                {
                    /* Do nothing*/
                }
            }
        }
    }
}


void draw_string(Framebuffer *fb, Rect_st *rect, const unsigned char *str, Enum_FontSize FontSize) 
{

    FontSize_st size;
    uint8_t jump_step = 0;


    switch (FontSize)
    {
        case FontSize_1608:
            size.height = (uint8_t)16;
            size.width = (uint8_t)8;
            jump_step = (uint8_t)8;

            break;
        case FontSize_2012:
            size.height = (uint8_t)20;
            jump_step = (uint8_t)12;
            size.width = (uint8_t)12;

            break;
        default:
            size.height = (uint8_t)16;
            jump_step = (uint8_t)8;
            size.width = (uint8_t)8;

            break;
        
    }

    while(*str != '\0')
    {       
        if((rect->x < fb->width) && (rect->y < fb->height))
        {
            draw_char(fb, *str, rect, size); 
            rect->x += jump_step;  // Increased spacing for row font
            str++;
        }
        else
        {
            break;  // Stop if off-screen
        }
    } 
}

void draw_char(Framebuffer *fb, unsigned char str, Rect_st *rect ,FontSize_st size) 
{
    uint16_t px,py, temp = 0;  
    uint16_t  dst_x, dst_y = 0;
    uint32_t color, dst_idx = 0; 
    uint8_t   red, green, blue, alpha = 0; 
    uint16_t bit_mask = 0;
    for( px = 0; px < size.height; px++ )//length
    {
        switch (size.height)
        {
            case 16:
                temp = ASCII_1608[str - 0x20][px];	
                bit_mask = 0x80;
                break;
            case 20:
                temp = (ASCII_2012[str - 0x20][px*2] << 8) | ASCII_2012[str - 0x20][px*2 + 1];
                bit_mask = 0x8000;
                break;            
            default:
                break;
        }

        for( py=0; py < size.width; py++)		// For each bit in this row
        {
            // Check if this bit is set in the font bitmap (read from MSB to LSB)
            if(temp & (bit_mask >> py)) 
            {
                // Draw pixel in red
                color = 0x00FF0000;
            }
            else
            {
                color = 0;
            }
            dst_x = rect->x + py;  // Column position
            dst_y = rect->y + px;  // Row position
            
            if (dst_x >= 0 && dst_x < (uint16_t)fb->width && 
                dst_y >= 0 && dst_y < (uint16_t)fb->height) 
            {
                dst_idx = (dst_y * fb->width + dst_x) * (fb->bpp / 8);
                
                // Draw pixel to framebuffer
                *((uint32_t*)(fb->buffer + dst_idx)) = color;
            }
        }
    }	
}