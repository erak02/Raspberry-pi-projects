🎯 Project Idea & Logic

The main idea of the project is:

Each time a car passes, the PIR sensor is triggered

The car counter is incremented

The total number of cars is displayed on the LCD

A red LED turns ON for 5 seconds, indicating that:

the current car is still passing

no new car should be counted during this period

After 5 seconds:

the red LED turns OFF

the green LED turns ON

the system is ready to count the next car

This ensures reliable counting and prevents multiple triggers from a single vehicle.

🔴🟢 LED Status Meaning
LED	Meaning
🟢 Green LED ON	System is ready to detect a new car
🔴 Red LED ON	Car detected – counting locked for 5 seconds
🧠 Key Embedded Concepts Used

GPIO interrupts (IRQ)

Shared ISR for multiple GPIO pins

volatile variables for ISR ↔ main loop communication

One-shot hardware alarms (add_alarm_in_ms)

Alarm cancellation to prevent overlapping timers

I2C communication

16x2 LCD control in 4-bit mode

Event-driven (interrupt-based) design

🧰 Hardware Components

Raspberry Pi Pico

PIR motion sensor

16x2 I2C LCD display (PCF8574)

Green LED

Red LED

Push button (reset counter)

Resistors (LED current limiting)

🔌 Pin Configuration
Component	GPIO Pin
LCD SDA	GPIO 0
LCD SCL	GPIO 1
PIR Sensor	GPIO 3
Reset Button	GPIO 4
Green LED	GPIO 5
Red LED	GPIO 6
⏱️ Timing Logic

When a car is detected:

a 5-second alarm is started

during this time, additional PIR triggers are ignored

The alarm callback restores the system to the ready state

Only one alarm can be active at a time

This guarantees deterministic behavior and prevents race conditions.

📟 LCD Output

The LCD displays:

Number of cars
detected: X

Where X is the total number of detected vehicles.

🔄 Reset Functionality

Pressing the reset button:

resets the car counter to 0

updates the LCD immediately

🧩 Code Structure Highlights

ISR (gpio_irq_handler)

Handles both PIR sensor and reset button

Keeps logic minimal and non-blocking

Alarm callback

Handles LED timing

Resets system state after 5 seconds

Main loop

Updates LCD only when needed (flag-based)

🚀 Possible Improvements

Debounce logic for PIR sensor

Non-blocking LCD updates using a state machine

EEPROM / Flash storage for persistent car count

Second sensor for direction detection (IN / OUT)

Power optimization using sleep modes

📄 License

This project is intended for educational and learning purposes.