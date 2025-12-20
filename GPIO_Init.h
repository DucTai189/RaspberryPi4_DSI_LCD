
#ifndef GPIO_INIT_H
#define GPIO_INIT_H

#include <stdio.h>
#include <unistd.h>
#include <gpiod.h>

#define E_OK 0u
#define E_NOT_OK 1u

#define CONSUMER "BlinkApp"
#define GPIO_CHIP "/dev/gpiochip0"
#define GPIO_LINE_16 16
#define GPIO_LINE_13 13
#define GPIO_LINE_20 20

extern struct gpiod_line *line_GPIO16;
extern struct gpiod_line *line_GPIO13;
extern struct gpiod_line *line_GPIO20;

__uint8_t Init_GPIO();

#endif // GPIO_INIT_H    