// display_config.h - Display configuration constants
#ifndef DISPLAY_CONFIG_H
#define DISPLAY_CONFIG_H
#include <stdio.h>
// 5" DSI Display Specifications (800x480)
#define DISPLAY_WIDTH 800
#define DISPLAY_HEIGHT 480
#define DISPLAY_BPP 32  // Bits per pixel

// DSI Configuration
#define DSI_LANES 2
#define DSI_FORMAT 0x1E  // RGB888

// Touchscreen calibration
typedef struct {
    int x_min;
    int x_max;
    int y_min;
    int y_max;
    int swap_axes;
    int invert_x;
    int invert_y;
} TouchCalibration;

// Default calibration values
#define TOUCH_RAW_MIN 0
#define TOUCH_RAW_MAX 4095
#define CALIBRATION_FILE "/tmp/touch_calibration.dat"

#endif