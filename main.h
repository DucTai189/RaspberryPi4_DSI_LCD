#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "Touchscreen.h"
#include "GPIO_Init.h"
#include <linux/input.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#define E_OK 0u
#define E_NOT_OK 1u