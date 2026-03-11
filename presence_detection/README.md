# Presence Detection Break Timer with I2C LCD

This project implements a presence-based work timer on the Raspberry Pi Pico using a PIR motion sensor, three LEDs, a push button, and a 16x2 I2C LCD display.

## Features

* Monitors user presence using a PIR motion sensor
* Counts down a 30-second activity timer while presence is detected
* Automatically switches to a pause state when the timer expires
* Detects absence if no motion is detected for 10 seconds
* Displays remaining time on a 16x2 LCD screen
* Uses three LEDs to indicate system state:

  * Green LED – active working period
  * Yellow LED – pause period
  * Red LED – user not present
* Push button allows the user to resume activity when the system is inactive
* Uses a repeating hardware timer (`add_repeating_timer_ms`) for time tracking
* Implements a Finite State Machine (FSM) for clean state transitions
* I2C communication drives the LCD in 4-bit mode

## Hardware Setup

* **Raspberry Pi Pico** microcontroller
* **PIR motion sensor**
* **16x2 character LCD display** with I2C interface (address 0x27)
* **Green LED** on GPIO 18
* **Yellow LED** on GPIO 17
* **Red LED** on GPIO 16
* **Push button** on GPIO 19
* **PIR sensor output** on GPIO 2
* I2C lines on GPIO 0 (SDA) and GPIO 1 (SCL)
* Resistors for LED current limiting
* Internal pull-up/pull-down resistors enabled for button and PIR sensor input

## Software Details

* Written in C using the Raspberry Pi Pico SDK
* LCD controlled through I2C using custom 4-bit communication routines
* A repeating timer interrupt updates time and monitors PIR activity every second
* A simple Finite State Machine manages system behavior:

  * `ACTIVE`
  * `PAUSE`
  * `NOT_ACTIVE`
* Events such as `TIME_ELAPSED`, `NO_PIR_DETECTION`, and `BUTTON_PRESSED` trigger state transitions
* LCD updates occur in the main loop using a screen update flag to avoid unnecessary redraws
* Button presses are handled using GPIO interrupts

## Notes

* The system transitions to **NOT_ACTIVE** if no motion is detected for 10 seconds
* When the activity timer reaches zero, the system enters a **10-second pause**
* The button allows the user to resume the active state when inactive
* The FSM design makes the code easy to extend with additional states or sensors
* The project can be extended with features like adjustable timers, sound notifications, or data logging
