#include "main.h"
#include <time.h>

#define BUTTON_IMAGE      "ICON_LIGHT_DEN.bmp"
#define BUTTON_ON_IMAGE   "ICON_LIGHT_XANH_LA.bmp"



int main(void) 
{
    Framebuffer fb;
    Image button_img;
    uint8_t return_value ;
    struct input_event ev;
    int touch_x = 0, touch_y = 0;
    bool led_state = 0;
    Rect_st btn_rect;

    
    if (init_framebuffer(&fb) < 0) 
    {
        fprintf(stderr, "Failed to initialize framebuffer\n");
        return 1;
    }
    
    if (load_image(BUTTON_IMAGE, &button_img) < 0) 
    {
        fprintf(stderr, "Failed to load button image\n");
        munmap(fb.buffer, fb.size);
        close(fb.fd);
        return 1;
    }
    return_value = Init_GPIO();
    gpiod_line_request_output(line_GPIO13, CONSUMER, 0);

    if (E_NOT_OK == return_value) 
    {
        perror("Failed to open GPIO chip");
       // stbi_image_free(button_img.pixels);
        munmap(fb.buffer, fb.size);
        close(fb.fd);
        return 1;
    }
    
    

    //Increase the size of image
    btn_rect.width = button_img.width + 30 ;
    //Increase the size of image
    btn_rect.height = button_img.height + 30;
    //Add the image in the center
    btn_rect.x = (fb.width - btn_rect.width) / 2;
    btn_rect.y = (fb.height - btn_rect.height) / 2;
    
    clear_screen(&fb, 0x000000);

    draw_image(&fb, &button_img, &btn_rect);
    
    
    int touch_fd = open(TOUCH_DEVICE, O_RDONLY);
    if (touch_fd < 0) 
    {
        touch_fd = open("/dev/input/touchscreen", O_RDONLY);
        if (touch_fd < 0) 
        {
            perror("Cannot open touch device");
            munmap(fb.buffer, fb.size);
            close(fb.fd);
            return 1;
        }
    }
    const unsigned char str[]="Turn on/off the LED by touching the button" ;

    draw_string(&fb,0,0, str, FontSize_2012) ;
    while (1) 
    {
        // Blocking read - waits for touch event
        if (read(touch_fd, &ev, sizeof(ev)) == sizeof(ev)) 
        {

            switch (ev.type) 
            {
                
                // Detect the touch position when touch event is happening
                case EV_ABS:
                    if (ev.code == ABS_X) 
                    {
                        touch_x = ev.value;
                    } 
                    else if (ev.code == ABS_Y)
                    {
                        touch_y = ev.value;
                    }
                  //  flag_touched = 1 ;
                    break;
                // Handing the touch event after completing touch event    
                case EV_KEY:
  

                    if (ev.code == BTN_TOUCH) 
                    {
                                          
                        // Check if touch is within button bounds
                        if ((touch_x >= btn_rect.x) && 
                            (touch_x <= btn_rect.x + btn_rect.width) &&
                            (touch_y >= btn_rect.y) && 
                            (touch_y <= btn_rect.y + btn_rect.height)) 
                            {            

                                // Update last touch time   
                                led_state = led_state ^ 1;
                                gpiod_line_set_value(line_GPIO13, led_state);
                                if (led_state == 1)
                                {
        
                                    load_image(BUTTON_ON_IMAGE, &button_img) ;
                                        //Increase the size of image
                                    btn_rect.width = button_img.width + 30 ;
                                        //Increase the size of image
                                    btn_rect.height = button_img.height + 30;
                                    //Add the image in the center
                                    btn_rect.x = (fb.width - btn_rect.width) / 2;
                                    btn_rect.y = (fb.height - btn_rect.height) / 2;
                                    draw_image(&fb, &button_img, &btn_rect);
                                }
                                else
                                {

                                    load_image(BUTTON_IMAGE, &button_img) ;
                                    //Increase the size of image
                                    btn_rect.width = button_img.width + 30 ;
                                    //Increase the size of image
                                    btn_rect.height = button_img.height + 30;
                                    //Add the image in the center
                                    btn_rect.x = (fb.width - btn_rect.width) / 2;
                                    btn_rect.y = (fb.height - btn_rect.height) / 2;
                                    draw_image(&fb, &button_img, &btn_rect);
                                }
                                
                            }
                            else
                            {

                            }
                        
                        // Reset coordinates after processing
                        touch_x = 0;
                        touch_y = 0;                                                 
                        
                    }
                    else
                    {
                        //Do nothing
                    }
                    break;
            }
        }
    }
    
    close(touch_fd);
    munmap(fb.buffer, fb.size);
    close(fb.fd);
    
    return 0;
}
