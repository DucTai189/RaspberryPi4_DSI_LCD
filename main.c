#include "main.h"
#include <poll.h>
#include <signal.h>
#include <pthread.h>
#include <time.h>   // For timer functions
#include <sched.h>  // For setting thread priority

#define BUTTON_IMAGE      "ICON_LIGHT_DEN.bmp"
#define BUTTON_ON_IMAGE   "ICON_LIGHT_XANH_LA.bmp"

pthread_mutex_t LOCK_DSI_CONTROL = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  COND_DSI_CONTROL = PTHREAD_COND_INITIALIZER;
pthread_mutex_t LOCK_TOUCH_DATA = PTHREAD_MUTEX_INITIALIZER;

volatile sig_atomic_t running = 1;

static Image button_img;
static Rect_st btn_rect;
static Rect_st text_rect = {0, 0, 0, 0};
static int touch_fd;
static Framebuffer fb;
volatile uint16_t touch_x = 0, touch_y = 0;
struct input_event ev;
volatile bool led_state = 0;
static void Handler_Display(int *touch_fd, Framebuffer *fb);
static void Handler_TouchEvent(int *touch_fd, Framebuffer *fb);

void signal_handler(int signum) 
{
    running = 0;
    // Cleanup GPIO resources
    Cleanup_GPIO();
}

static volatile bool touch_ready = false;
void* Task_Handler_Display(void* arg) 
{
    while (1) 
    {
        pthread_mutex_lock(&LOCK_DSI_CONTROL);
        pthread_mutex_lock(&LOCK_TOUCH_DATA);
        while (!touch_ready) 
        {
            pthread_cond_wait(&COND_DSI_CONTROL, &LOCK_DSI_CONTROL);  // Wait for timer to signal
            pthread_mutex_unlock(&LOCK_TOUCH_DATA);
        }
        touch_ready = false;
       // pthread_mutex_unlock(&LOCK_TOUCH_DATA);
        // Handle DSI display update here
        Handler_Display(&touch_fd, &fb);

        pthread_mutex_unlock(&LOCK_DSI_CONTROL);

    }
    return NULL;
}

void timer_handler(int sig, siginfo_t* si, void* uc) 
{
    int id = *(int*)si->si_value.sival_ptr;

    switch (id)
    {
        case 1:
            pthread_cond_signal(&COND_DSI_CONTROL);
        break;

        default:
        break;
    }

}
void create_timer(timer_t* timerid, int signal_id, int ms_period) 
{
    struct sigevent sev;
    struct itimerspec its;

    sev.sigev_notify = SIGEV_SIGNAL;
    sev.sigev_signo = SIGRTMIN;
    sev.sigev_value.sival_ptr = malloc(sizeof(int));
    *(int*)(sev.sigev_value.sival_ptr) = signal_id;
    timer_create(CLOCK_REALTIME, &sev, timerid);

    its.it_value.tv_sec = ms_period / 1000;
    its.it_value.tv_nsec = (ms_period % 1000) * 1000000;
    its.it_interval = its.it_value;

    timer_settime(*timerid, 0, &its, NULL);
}

static void Handler_Display(int *touch_fd, Framebuffer *fb)
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
        btn_rect.x = (fb->width - btn_rect.width) / 2;
        btn_rect.y = (fb->height - btn_rect.height) / 2;
        draw_image(fb, &button_img, &btn_rect);
    }
    else
    {

        load_image(BUTTON_IMAGE, &button_img) ;
        //Increase the size of image
        btn_rect.width = button_img.width + 30 ;
        //Increase the size of image
        btn_rect.height = button_img.height + 30;
        //Add the image in the center
        btn_rect.x = (fb->width - btn_rect.width) / 2;
        btn_rect.y = (fb->height - btn_rect.height) / 2;
        draw_image(fb, &button_img, &btn_rect);
    }
}
void Handler_TouchEvent(int *touch_fd, Framebuffer *fb)
{
    if (read(*touch_fd, &ev, sizeof(ev)) == sizeof(ev)) 
    {
        switch (ev.type) 
        {
            // Detect the touch position when touch event is happening
            case EV_ABS:
            pthread_mutex_lock(&LOCK_TOUCH_DATA);
                if (ev.code == ABS_X) 
                {
                    touch_x = ev.value;
                } 
                else if (ev.code == ABS_Y)
                {
                    touch_y = ev.value;
                }
            pthread_mutex_unlock(&LOCK_TOUCH_DATA);
                break;
            // Handing the touch event after completing touch event    
            case EV_KEY:
            pthread_mutex_lock(&LOCK_TOUCH_DATA);
                if (ev.code == BTN_TOUCH) 
                {
                                        
                    // Check if touch is within button bounds
                    if ((touch_x >= btn_rect.x) && 
                        (touch_x <= btn_rect.x + btn_rect.width) &&
                        (touch_y >= btn_rect.y) && 
                        (touch_y <= btn_rect.y + btn_rect.height)) 
                        {            
                            touch_ready = true;
                          //  pthread_cond_signal(&COND_DSI_CONTROL);  // Signal the display handler
                            
                        }
                        else
                        {

                        }
                    
                    // Reset coordinates after processing
                    touch_x = 0;
                    touch_y = 0;    
                    pthread_mutex_unlock(&LOCK_TOUCH_DATA);                                             
                }
                else
                {
                    //Do nothing
                }
            break;
        }
    }
}
int main(void) 
{

    uint8_t return_value ;
    // Setup poll structure
    struct pollfd fds[1];
    // Setup signal
    struct sigaction sa;
    // Start threads
    pthread_t thread_DSI_CONTROL;
    // Create timers
    timer_t timer_DSI_CONTROL_Thread;

    pthread_create(&thread_DSI_CONTROL, NULL, Task_Handler_Display, NULL);
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = timer_handler;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGRTMIN, &sa, NULL);
    
    signal(SIGINT, signal_handler);

    const unsigned char str[]="Turn on/off the LED by touching the button" ;
    
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
    //return_value = gpiod_line_request_output(line_GPIO13, CONSUMER, 0);

    if (E_OK != return_value) 
    {
        perror("Failed to open GPIO chip");
        munmap(fb.buffer, fb.size);
        close(fb.fd);
        return 1;
    }

    touch_fd = open(TOUCH_DEVICE, O_RDONLY);
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

    clear_screen(&fb, 0x000000); // Clear screen to black

    // Draw the position of string on the screen
    text_rect.x = 120 ;
    text_rect.y = 0 ;
    draw_string(&fb, &text_rect, str, FontSize_2012); 
    //Increase the size of image
    btn_rect.width = button_img.width + 30 ;
    //Increase the size of image
    btn_rect.height = button_img.height + 30;
    //Add the image in the center
    btn_rect.x = (fb.width - btn_rect.width) / 2;
    btn_rect.y = (fb.height - btn_rect.height) / 2;

    draw_image(&fb, &button_img, &btn_rect);

    fds[0].fd = touch_fd;
    fds[0].events = POLLIN;
    int count=0;
    create_timer(&timer_DSI_CONTROL_Thread, 1, 50);   // Task Display LCD  period: 50 ms
    while (running) 
    {
        // Block until touch event (interrupt-driven)
        return_value = poll(fds, 1, 50);  // 1 sec timeout
        
        if (return_value < 0) 
        {
            perror("Cannot poll touch device");

        }
        
        if (return_value == 0) 
        {
            // Timeout occurred, no touch event
            count++;
            printf("No touch event detected within timeout period. Count: %d\n", count);
        }        
  
        if (fds[0].revents & POLLIN) 
        {
            // Now read the event
            // Blocking read - waits for touch event
            Handler_TouchEvent(&touch_fd, &fb);
        }

    }
    
    // Cleanup resources before exit
    Cleanup_GPIO();
    close(touch_fd);
    munmap(fb.buffer, fb.size);
    close(fb.fd);
    
    return 0;
}
