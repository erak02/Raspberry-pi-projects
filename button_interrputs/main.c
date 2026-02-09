#include "pico/stdlib.h"
#include "hardware/gpio.h"

const uint LED_PIN=16;
const uint BUTTON_ON=20;
const uint BUTTON_OFF=17;

void button_irq_handler(uint gpio, uint32_t events)
{
    if(gpio == BUTTON_ON && (events & GPIO_IRQ_EDGE_FALL))
    {
        gpio_put(LED_PIN,1);
    }
    else if (gpio == BUTTON_OFF && (events & GPIO_IRQ_EDGE_RISE))
    {
        gpio_put(LED_PIN,0);
    }
}

int main()
{
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    gpio_init(BUTTON_ON);
    gpio_set_dir(BUTTON_ON, GPIO_IN);
    gpio_pull_up(BUTTON_ON);

    gpio_init(BUTTON_OFF);
    gpio_set_dir(BUTTON_OFF, GPIO_IN);
    gpio_pull_up(BUTTON_OFF);

    gpio_set_irq_enabled_with_callback(BUTTON_ON, GPIO_IRQ_EDGE_FALL, true, &button_irq_handler);
    gpio_set_irq_enabled_with_callback(BUTTON_OFF, GPIO_IRQ_EDGE_RISE, true, &button_irq_handler);
    
    while(true)
    {
        tight_loop_contents();
    }
}