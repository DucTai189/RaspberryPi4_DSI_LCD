# Raspberry Pi 4 DSI Touchscreen Controller

A C-based touchscreen application for Raspberry Pi 4 with DSI display that controls GPIO pins through an interactive touch interface.

## 📋 Overview

This project provides a graphical touchscreen interface that displays a toggleable button. When pressed, it controls an LED connected to GPIO pin 13. The application uses direct framebuffer access for rendering and Linux input events for touch handling.

## ✨ Features

- **Direct Framebuffer Rendering** - Hardware-accelerated display output via `/dev/fb0`
- **Touch Input Handling** - Linux input event processing with debouncing
- **GPIO Control** - Toggle LED on GPIO pin 13 using libgpiod
- **Image-based UI** - BMP image support for button graphics
- **Touch Debouncing** - 300ms debounce filter to prevent accidental double-touches
- **Centered UI Layout** - Automatically centers button on any screen resolution

## 🛠️ Hardware Requirements

- Raspberry Pi 4
- Official Raspberry Pi DSI Touchscreen (or compatible)
- LED connected to GPIO pin 13
- (Optional) Additional components on GPIO pins 16 and 20

## 📦 Software Dependencies

Install the required libraries:

```bash
sudo apt-get update
sudo apt-get install -y \
    libgpiod-dev \
    libsdl2-dev \
    libsdl2-image-dev \
    build-essential
```

## 🗂️ Project Structure

```
RaspPi4_DSI_TouchScreen/
├── main.c              # Main application entry point
├── main.h              # Main header with includes
├── GPIO_Init.c         # GPIO initialization functions
├── GPIO_Init.h         # GPIO definitions and declarations
├── Touchscreen.c       # Framebuffer and image handling
├── Touchscreen.h       # Display structure definitions
├── stb_image.h         # STB image loading library (header-only)
├── make.mk             # Makefile for building the project
├── ICON_LIGHT_DEN.bmp  # Button OFF image (dark)
├── ICON_LIGHT_XANH_LA.bmp  # Button ON image (green/light)
└── README.md           # This file
```

## 🚀 Building the Project

### Build

```bash
make -f make.mk all
```

### Clean

```bash
make -f make.mk clean
```

### Debug Build

```bash
DEBUG=1 make -f make.mk all
```

## ▶️ Running the Application


1. Run the application:
```bash
./DSI_LCD
```

Or with sudo if permissions aren't set:
```bash
sudo ./DSI_LCD
```

2. Touch the button on screen to toggle the LED on GPIO pin 13

3. Press `Ctrl+C` to exit

## 🎮 Usage

- The application displays a centered button on the touchscreen
- **Touch the button** to toggle the LED state
- **Red/Dark button** = LED OFF
- **Green/Light button** = LED ON
- Touch debouncing prevents accidental double-presses (300ms filter)

## ⚙️ Configuration

### GPIO Pin Configuration

Edit [GPIO_Init.h](GPIO_Init.h) to change GPIO pins:

```c
#define GPIO_LINE_13 13  // Main LED control pin
#define GPIO_LINE_16 16  // Additional GPIO (if needed)
#define GPIO_LINE_20 20  // Additional GPIO (if needed)
```

### Touch Debounce Time

Edit [main.c](main.c) line 5:

```c
#define DEBOUNCE_MS 300  // Change to adjust sensitivity (milliseconds)
```

### Button Images

Replace these files with your own BMP images:
- `ICON_LIGHT_DEN.bmp` - Button OFF state
- `ICON_LIGHT_XANH_LA.bmp` - Button ON state

Update [main.c](main.c) lines 4-5 to use different image files.

## 🔧 Troubleshooting

### GPIO Resource Busy

If you get "resource busy" errors:

```bash
# Kill any running instances
sudo pkill DSI_LCD

# Force GPIO release
gpioset gpiochip0 13=0
```

### Touch Not Working

Check available input devices:
```bash
ls -l /dev/input/
cat /proc/bus/input/devices
```

Update `TOUCH_DEVICE` in [Touchscreen.h](Touchscreen.h) if needed.

### Display Issues

Verify framebuffer is available:
```bash
ls -l /dev/fb0
fbset -i
```

### LED Always On/Off

Your LED might be wired active-low. Invert the logic in [main.c](main.c):


## 📚 Code Architecture

### Main Components

1. **Framebuffer Management** ([Touchscreen.c](Touchscreen.c))
   - `init_framebuffer()` - Initialize display
   - `draw_image()` - Render images with scaling
   - `clear_screen()` - Fill screen with color

2. **GPIO Control** ([GPIO_Init.c](GPIO_Init.c))
   - `Init_GPIO()` - Setup GPIO pins using libgpiod
   - Controls GPIO pins 13, 16, 20

3. **Touch Input** ([main.c](main.c))
   - Event loop processing `EV_ABS` (position) and `EV_KEY` (press/release)
   - Time-based debouncing with `CLOCK_MONOTONIC`
   - Touch coordinate validation

4. **Image Loading** ([Touchscreen.c](Touchscreen.c))
   - Uses STB image library for BMP/PNG support
   - RGBA pixel format handling
   - Alpha blending support

## 🔍 How Touch Events Work

1. **EV_ABS** events report touch coordinates (X, Y)
2. **EV_KEY** events report touch state (pressed/released)
3. Coordinates are validated against button boundaries
4. Debounce filter prevents multiple triggers within 300ms
5. Valid touches toggle LED state and update button image

## 📄 License

This project is provided as-is for educational and personal use.

## 🙏 Credits

- **STB Image Library** - Sean Barrett (public domain image loader)
- **libgpiod** - Linux GPIO character device library

## 📧 Support

For issues or questions, check:
- GPIO pin connections and wiring
- System permissions (`gpio` and `video` groups)
- Input device paths in code match your hardware
- Console output for debugging information

---

**Note:** This application requires direct hardware access. Always run with appropriate permissions and ensure proper electrical connections to avoid hardware damage.
