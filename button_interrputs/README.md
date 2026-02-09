# LED Control with Buttons using GPIO Interrupts – Raspberry Pi Pico

This project demonstrates controlling an LED connected to Raspberry Pi Pico using two buttons with GPIO interrupts.

## Functionality
- Pressing the **BUTTON_ON** (GPIO 20) turns the LED (GPIO 16) ON.
- Pressing the **BUTTON_OFF** (GPIO 17) turns the LED OFF.
- Uses GPIO interrupts to respond immediately to button presses.

## Hardware Connections
- **LED** connected to GPIO 16 (with appropriate resistor)
- **BUTTON_ON** connected to GPIO 20 with pull-up resistor enabled internally
- **BUTTON_OFF** connected to GPIO 17 with pull-up resistor enabled internally

## Software Details
- Written in C using the Pico SDK
- Uses GPIO IRQ handlers to detect button edge events
- BUTTON_ON triggers on falling edge (button press)
- BUTTON_OFF triggers on rising edge (button release)

