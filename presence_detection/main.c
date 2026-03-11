#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include <stdio.h>
#include "hardware/timer.h"
#include <stdlib.h>

#define I2C_PORT i2c0

const uint SDA_PIN = 0;
const uint SCL_PIN = 1;

#define LCD_ADDR 0x27

#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME 0x02
#define LCD_ENTRYMODESET 0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_FUNCTIONSET 0x20
#define LCD_SETDDRAMADDR 0x80

#define LCD_5x10DOTS 0x04
#define LCD_2LINE 0x08

#define LCD_DISPLAYON 0x04

#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYLEFT 0x02

#define LCD_BACKLIGHT 0x08
#define LCD_ENABLE 0x04
#define LCD_RS 0x01

const uint GREEN_LED = 18;
const uint YELLOW_LED = 17;
const uint RED_LED = 16;
const uint PIR_PIN = 2;
const uint BUTTON = 19;

volatile int time = 30;
volatile int presence_time = 0;

char time_string[5];

volatile bool update_screen;
volatile bool start = false;

typedef enum{
    ACTIVE,
    NOT_ACTIVE,
    PAUSE
} State;

typedef enum{
    NONE,
    TIME_ELAPSED,
    NO_PIR_DETECTION,
    BUTTON_PRESSED
} Event;

State current_state = ACTIVE;
volatile Event pending_event = NONE;

void lcd_clear();
void lcd_set_cursor(int line, int position);
void lcd_string(char s[]);

void enter_active()
{
    lcd_clear();
    lcd_set_cursor(0,0);
    lcd_string("Time left until");
    lcd_set_cursor(1,0);
    lcd_string("pause: ");

    gpio_put(YELLOW_LED, 0);
    gpio_put(GREEN_LED, 1);
    gpio_put(RED_LED, 0);
}

void enter_pause()
{
    lcd_clear();
    lcd_set_cursor(0,0);
    lcd_string("Pause for: ");

    gpio_put(GREEN_LED, 0);
    gpio_put(YELLOW_LED, 1);
    gpio_put(RED_LED, 0);
}

void enter_not_active()
{
    lcd_clear();
    lcd_set_cursor(0,0);
    lcd_string("Press button to");
    lcd_set_cursor(1,0);
    lcd_string("continue!");

    gpio_put(GREEN_LED, 0);
    gpio_put(YELLOW_LED, 0);
    gpio_put(RED_LED, 1);
}

void handle_event (Event event)
{
    switch (current_state)
    {
        case ACTIVE:
            switch (event)
            {
                case TIME_ELAPSED:
                {
                    current_state = PAUSE;
                    time = 10;
                    enter_pause();
                    break;
                }
                case NO_PIR_DETECTION:
                {
                    current_state = NOT_ACTIVE;
                    presence_time = 0;
                    enter_not_active();
                    break;
                }
                default:
                    break;
            }
            break;
        case NOT_ACTIVE:
            switch (event)
            {
                case BUTTON_PRESSED:
                {
                    current_state = ACTIVE;
                    presence_time = 0;
                    enter_active();
                    break;
                }
                default:
                    break;
            }
            break;
        case PAUSE:
            switch (event)
            {
                case TIME_ELAPSED:
                {
                    current_state = ACTIVE;
                    presence_time = 0;
                    time = 30;
                    enter_active();
                    break;
                }
                default:
                    break;
            }
            break;
    }
}

void i2c_write_byte(uint8_t data)
{
    i2c_write_blocking(I2C_PORT, LCD_ADDR, &data, 1, false);
}

void lcd_pulse_enable(uint8_t data)
{
    i2c_write_byte(data | LCD_ENABLE);
    sleep_us(500);

    i2c_write_byte(data &~ LCD_ENABLE);
    sleep_us(500);
}

void lcd_send_nibble(uint8_t nibble, uint8_t rs)
{
    uint8_t data = 0;
    data = data | (nibble & 0xF0);
    data = data | LCD_BACKLIGHT;

    if (rs)
    {
        data = data | LCD_RS;
    }

    i2c_write_byte(data);
    lcd_pulse_enable(data);
}

void lcd_write_byte(uint8_t val, uint8_t rs)
{
    lcd_send_nibble(val & 0xf0, rs);
    lcd_send_nibble((val << 4) & 0xf0, rs);
}

void lcd_init()
{
    sleep_ms(50);

    lcd_send_nibble(0x30, 0);
    sleep_us(150);

    lcd_send_nibble(0x30, 0);
    sleep_us(150);

    lcd_send_nibble(0x30, 0);
    sleep_us(150);

    lcd_send_nibble(0x20, 0);
    sleep_us(150);

    lcd_write_byte(LCD_FUNCTIONSET | LCD_2LINE, 0);
    lcd_write_byte(LCD_DISPLAYCONTROL | LCD_DISPLAYON, 0);
    lcd_write_byte(LCD_CLEARDISPLAY, 0);
    sleep_ms(2);
    lcd_write_byte(LCD_ENTRYMODESET | LCD_ENTRYLEFT, 0);
}

void lcd_set_cursor(int line, int position)
{
    if (position > 15)
    {
        position = 15;
    }

    int value;

    if (line == 0)
    {
        value = LCD_SETDDRAMADDR | position;
    }
    else if (line == 1)
    {
        value = LCD_SETDDRAMADDR | (position + 0x40);
    }

    lcd_write_byte(value, 0);
}

void lcd_char(char val)
{
    lcd_write_byte(val, 1);
}

void lcd_string(char s[])
{
    for (int i = 0; s[i] != '\0'; i++)
    {
        lcd_char(s[i]);
    }
}

void lcd_clear()
{
    lcd_write_byte(LCD_CLEARDISPLAY, 0);
    sleep_ms(2);
}

bool repeating_timer_callback(struct repeating_timer *t)
{
    if (current_state != NOT_ACTIVE && time > 0)
    {
        time--;
    }

    if (current_state == ACTIVE)
    {
        if (gpio_get(PIR_PIN))
        {
            presence_time = 0;
        }
        else
        {
            presence_time++;
        }

        if (presence_time >= 10 && pending_event == NONE)
        {
            pending_event = NO_PIR_DETECTION;
        }
    }

    if (time <= 0 && pending_event == NONE)
    {
        pending_event = TIME_ELAPSED;
    }

    update_screen = true;

    return true;
}

void gpio_irq_handler(uint gpio, uint32_t events)
{
    if (gpio == BUTTON && (events & GPIO_IRQ_EDGE_FALL))
    {
             pending_event = BUTTON_PRESSED;
    }
}

int main()
{   
    stdio_init_all();

    i2c_init(I2C_PORT, 100*1000);
    
    gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(SDA_PIN);
    gpio_pull_up(SCL_PIN);

    lcd_init();

    gpio_init(GREEN_LED);
    gpio_set_dir(GREEN_LED, GPIO_OUT);
    gpio_init(YELLOW_LED);
    gpio_set_dir(YELLOW_LED, GPIO_OUT);
    gpio_init(RED_LED);
    gpio_set_dir(RED_LED, GPIO_OUT);

    gpio_init(PIR_PIN);
    gpio_set_dir(PIR_PIN, GPIO_IN);
    gpio_pull_down(PIR_PIN);

    gpio_init(BUTTON);
    gpio_set_dir(BUTTON, GPIO_IN);
    gpio_pull_up(BUTTON);

    struct repeating_timer timer;
    add_repeating_timer_ms(-1000, repeating_timer_callback, NULL, &timer);

    gpio_set_irq_enabled_with_callback(BUTTON, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);

    current_state = ACTIVE;
    enter_active();

    while(true)
    {
        tight_loop_contents();

        if (pending_event != NONE)
        {
            handle_event(pending_event);
            pending_event = NONE;
        }

        if (update_screen)
        {
            if (current_state == ACTIVE)
            {
                snprintf(time_string, sizeof(time_string), "%3d", time);
                lcd_set_cursor(1,7);
                lcd_string(time_string);
            }
            else if (current_state == PAUSE)
            {
                snprintf(time_string, sizeof(time_string), "%3d", time);
                lcd_set_cursor(0,10);
                lcd_string(time_string);
            }
            update_screen = false;
        }
    }
}