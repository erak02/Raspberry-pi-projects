#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include <string.h>
#include <stdio.h>

#define I2C_PORT i2c0

#define SDA_PIN 4
#define SCL_PIN 5

#define LCD_ADDR 0x27

/*komande LCD kontrolera*/
#define LCD_CLEARDISPLAY 0x01 //brise sav prikaz displaya i vraca kursor na pocetak
#define LCD_RETURNHOME 0x02 //postavlja kursor na pocetak bez brisanja displaya
#define LCD_ENTRYMODESET 0x04 //kako se kursor pomera nakon upisa karaktera
#define LCD_DISPLAYCONTROL 0x08 //ukljucivanje/iskljucivanje kursora, displaya i blinkanje kursora
#define LCD_CURSORSHIFT 0x10 //pomeranje kursora ili celog prikaza levo i desno
#define LCD_FUNCTIONSET 0x20 //definise broj linija, velicinu karaktera i rezim rada (4-bitni ili 8-bitni)
#define LCD_SETCGRAMADDR 0x40 //kreiranje i koriscenje custom simbola
#define LCD_SETDDRAMADDR 0x80 //pomera kursor na odredjenu poziciju na ekranu 

/*flags za funkciju (function set)*/
#define LCD_5x10DOTS 0x04 //postavlja font na 5x10 tackica po karakteru
#define LCD_2LINE 0x08 //postavlja LCD da koristi dve linije umesto jedne

/*flags za prikaz i kursor*/
#define LCD_BLINKON 0x01 //blinkanje kursora 
#define LCD_CURSORON 0x02 //prikazuje kursor na ekranu
#define LCD_DISPLAYON 0x04 //ukljucuje prikaz na ekranu, bez ovoga, ekran je prazan

/*flags za rezim unosa*/
#define LCD_ENTRYSHIFTINCREMENT 0x01 //kursor se pomera ulevo nakon unosa karaktera
#define LCD_ENTRYLEFT 0x02 //postavlja smer pomeranja kursora na levo nakon unosa karaktera

/*ako je bit LCD_BACKLIGHT 1, onda je backlight on*/
/*lcd ne cita podatke automatski, gleda D4-D7 i tek kada LCD_ENABLE predje sa 1 na 0 on zakljuca podatke*/
/*ako je LCD_RS 0, u pitanju je komanda (clear, cursor...), dok ako je LCD_RS 1 onda je ASCII karakter*/
#define LCD_BACKLIGHT 0x08
#define LCD_ENABLE 0x04
#define LCD_RS 0x01

const uint BUTTON_ON = 20;
const uint BUTTON_RESET = 15;

volatile bool button_on = false;
volatile bool button_off = false;
volatile bool screen_needs_update = false;

int current_lap=0;
char count_string[32];
int best_lap = 0;

/*data je jedan bajt koji salje, svaki bit tog bajta postavlja pinove na odgovarajuci nivo
koristi se blocking jer je display spor, CPU ceka dok se bajt stvarno ne posalje*/
void i2c_write_byte(uint8_t data)
{
    i2c_write_blocking(I2C_PORT, LCD_ADDR, &data, 1, false);
}

void lcd_pulse_enable(uint8_t data)
{
    i2c_write_byte(data | LCD_ENABLE);
    sleep_us(500);

    i2c_write_byte(data & ~LCD_ENABLE);
    sleep_us(500);
}

/*funkcija za slanje samo jednog nibblea*/
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

/*LCD radi u 4-bitnom rezimu, ASCII/komanda ima 8 bita pa zbog toga moram da saljem  iz dva puta
prvi put saljem prva 4 bita ASCII/komande i 4 kontrolna bita
drugi put saljem preostala 4 bita ASCII/komande i 4 kontrolna bita*/
void lcd_write_byte(uint8_t val, uint8_t rs)
{
    lcd_send_nibble(val & 0xF0, rs);
    /*shiftuje bitove za 4 mesta ulevo, bitovi koji ispadnu sa leve strane se odbacuju*/
    lcd_send_nibble((val<<4) & 0xF0, rs);
}

/*posalji 0x03 tri puta da resetujes LCD u 8-bitni mod
posalji 0x02 da predjes u 4-bitni mod*/
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
    if(position > 15)
    {
        position=15;
    }

    int value;

    if(line == 0)
    {
        value = LCD_SETDDRAMADDR | position;
    }
    else if(line == 1)
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
   for(int i=0; s[i]!='\0'; i++)
   {
        lcd_char(s[i]);
   }
}

void lcd_clear()
{
    lcd_write_byte(LCD_CLEARDISPLAY, 0);
    sleep_ms(2);
}

void button_irq_handler(uint gpio, uint32_t events)
{
    if(gpio == BUTTON_ON && (events & GPIO_IRQ_EDGE_FALL))
    {
        if (button_on == true)
        {
            screen_needs_update = true;
            button_on = false;
            button_off = true;
        }
        else if (button_on == false)
        {
            screen_needs_update = true;
            button_on = true;
            button_off = false;
        }
    }
    else if (gpio == BUTTON_RESET && (events & GPIO_IRQ_EDGE_FALL))
    {
        if (current_lap > best_lap)
            {
                best_lap = current_lap;
            }
        current_lap = 0;
        screen_needs_update = true;
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

    gpio_init(BUTTON_ON);
    gpio_set_dir(BUTTON_ON, GPIO_IN);
    gpio_pull_up(BUTTON_ON);

    gpio_init(BUTTON_RESET);
    gpio_set_dir(BUTTON_RESET, GPIO_IN);
    gpio_pull_up(BUTTON_RESET);

    gpio_set_irq_enabled_with_callback(BUTTON_ON, GPIO_IRQ_EDGE_FALL, true, &button_irq_handler);
    gpio_set_irq_enabled_with_callback(BUTTON_RESET, GPIO_IRQ_EDGE_FALL, true, &button_irq_handler);

    lcd_set_cursor(0,0);
    lcd_string("Press button to");
    lcd_set_cursor(1,0);
    lcd_string("start!");

    while(true)
    {
        tight_loop_contents();

        if(button_on == true)
        {
            if(screen_needs_update == true)
            {
                lcd_clear();

                lcd_set_cursor(0,0);            
                lcd_string("Current lap:");

                lcd_set_cursor(1,0);
                lcd_string("Best lap:");

                screen_needs_update = false;    
            }

            lcd_set_cursor(0, 13);
            snprintf(count_string, sizeof(count_string), "%d", current_lap);
            lcd_string(count_string);

            lcd_set_cursor(1,10);
            snprintf(count_string, sizeof(count_string), "%d", best_lap);
            lcd_string(count_string);

            sleep_ms(1000);
            current_lap++;
        }
        else if (button_off == true)
        {
            if(screen_needs_update == true)
            {
                lcd_clear();

                lcd_set_cursor(0, 0);
                lcd_string("Your best time: ");
                lcd_set_cursor(1, 0);
                snprintf(count_string, sizeof(count_string), "%d", best_lap);
                lcd_string(count_string);

                screen_needs_update = false;
            }             
        }
    }
}










