#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include <stdio.h>
#include "wheels.h"

#define BTN_20_PIN 20
#define BTN_21_PIN 21
#define BTN_22_PIN 22

float duty_cycle = 0.0f;

void init_gpio();
void irq_handler(uint gpio, uint32_t events);
void irq_btn(uint gpio);

int main() 
{
    init_gpio();

    while (true) 
    {
        tight_loop_contents();
    }
}

void init_gpio() 
{
    stdio_init_all();
    init_wheels();

    gpio_set_irq_enabled_with_callback(BTN_20_PIN, GPIO_IRQ_EDGE_FALL, true, &irq_handler);
    gpio_set_irq_enabled_with_callback(BTN_21_PIN, GPIO_IRQ_EDGE_FALL, true, &irq_handler);
    gpio_set_irq_enabled_with_callback(BTN_22_PIN, GPIO_IRQ_EDGE_FALL, true, &irq_handler);
}
void irq_handler(uint gpio, uint32_t events)
{
    if (gpio == BTN_20_PIN || gpio == BTN_21_PIN || gpio == BTN_22_PIN)
        irq_btn(gpio);
}
void irq_btn(uint gpio)
{
    if (gpio == BTN_20_PIN) 
    {
        set_car_state(CAR_FORWARD);
    }
    else if (gpio == BTN_21_PIN) 
    {
        if (duty_cycle >= 1.f)
            duty_cycle = 0.f;
        else
            duty_cycle = 1.f;

        set_motor_pwm_duty_cycle(WHEEL_LEFT_PWN_PIN, duty_cycle);
        set_motor_pwm_duty_cycle(WHEEL_RIGHT_PWN_PIN, duty_cycle);
    }
    else if (gpio == BTN_22_PIN) 
    {
        set_car_state(CAR_BACKWARD);
    }
}