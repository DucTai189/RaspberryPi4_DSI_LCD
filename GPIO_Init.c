#include "GPIO_Init.h"


struct gpiod_line *line_GPIO16;
struct gpiod_line *line_GPIO13;
struct gpiod_line *line_GPIO20;
struct gpiod_chip *gpio_chip = NULL;

__uint8_t Init_GPIO() 
{
    __uint8_t ret = E_OK;

    // Open the GPIO chip
    gpio_chip = gpiod_chip_open(GPIO_CHIP);

    if (!gpio_chip) 
    {
        perror("Open chip failed");
        ret = E_NOT_OK;
    }

    // Get the GPIO line
    line_GPIO13 = gpiod_chip_get_line(gpio_chip, GPIO_LINE_13);
    if (!line_GPIO13) 
    {
        perror("Get line failed");
        gpiod_chip_close(gpio_chip);
        ret = E_NOT_OK;
    }

    // Request the line as output
    ret = gpiod_line_request_output(line_GPIO13, CONSUMER, 0);
    if (ret < 0) {
        perror("Request line as output failed");
        gpiod_chip_close(gpio_chip);
        ret = E_NOT_OK;
    }

    // Get the second GPIO line
    line_GPIO16 = gpiod_chip_get_line(gpio_chip, GPIO_LINE_16);
    if (!line_GPIO16) 
    {
        perror("Get line failed");
        gpiod_chip_close(gpio_chip);
        ret = E_NOT_OK;
    }

    // Request the second line as output
    ret = gpiod_line_request_output(line_GPIO16, CONSUMER, 0);
    if (ret < 0) 
    {
        perror("Request line as output failed");
        gpiod_chip_close(gpio_chip);
        ret = E_NOT_OK;
    }

    return  ret ;
}

void Cleanup_GPIO()
{
    if (line_GPIO13) {
        gpiod_line_release(line_GPIO13);
    }
    if (line_GPIO16) {
        gpiod_line_release(line_GPIO16);
    }
    if (gpio_chip) {
        gpiod_chip_close(gpio_chip);
    }
}




