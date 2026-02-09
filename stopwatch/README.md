# Raspberry Pi Pico Stopwatch with I2C LCD Display

This project implements a simple stopwatch on the Raspberry Pi Pico using an I2C 16x2 LCD display and two buttons with interrupts.

## Features

- Start/stop stopwatch using a button (BUTTON_ON)
- Reset stopwatch and save best lap time using a reset button (BUTTON_RESET)
- Displays current lap time and best lap time on LCD
- Uses I2C communication to drive the LCD (address 0x27)
- Handles button presses via GPIO interrupts for responsive control

## Hardware Setup

- **Raspberry Pi Pico** microcontroller
- **16x2 character LCD display** with I2C interface (address 0x27)
- Buttons connected to GPIO pins 20 (start/stop) and 15 (reset)
- I2C lines on GPIO 4 (SDA) and GPIO 5 (SCL)
- Internal pull-up resistors enabled on button pins

## Software Details

- Written in C using the Pico SDK
- I2C communication uses blocking writes to ensure LCD sync
- LCD initialized and controlled in 4-bit mode using custom commands
- Uses GPIO IRQ handlers to detect button presses (falling edges)
- Current lap increments every second while running
- Best lap time saved and displayed

## Notes

- The LCD backlight is always on.
- Timing accuracy depends on the sleep interval and CPU load.
- The project can be extended with features like pause, split times, or custom symbols.
