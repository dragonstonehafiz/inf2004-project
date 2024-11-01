#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include <stdio.h>
#include "wheels.h"

#define BTN_DECREASE_SPEED 22
#define BTN_MOVE 21
#define BTN_INCREASE_SPEED 20

float duty_cycle = 0.5f;
bool isCarMoving = false;

void init_gpio();
void init_inerrupts();
void irq_handler(uint gpio, uint32_t events);

int main() 
{
    init_gpio();
    init_inerrupts();

    while (true) 
    {
        tight_loop_contents();
    }
}

void init_gpio() 
{
    stdio_init_all();

    gpio_init(BTN_DECREASE_SPEED);
    gpio_init(BTN_INCREASE_SPEED);
    gpio_init(BTN_MOVE);

    init_wheels();
    set_wheels_duty_cycle(duty_cycle);

}
void init_inerrupts()
{
    gpio_set_irq_enabled_with_callback(BTN_DECREASE_SPEED, GPIO_IRQ_EDGE_FALL, true, &irq_handler);
    gpio_set_irq_enabled_with_callback(BTN_MOVE, GPIO_IRQ_EDGE_FALL, true, &irq_handler);
    gpio_set_irq_enabled_with_callback(BTN_INCREASE_SPEED, GPIO_IRQ_EDGE_FALL, true, &irq_handler);
    gpio_set_irq_enabled_with_callback(WHEEL_ENCODER_RIGHT_PIN, GPIO_IRQ_EDGE_FALL, true, &irq_handler);
    gpio_set_irq_enabled_with_callback(WHEEL_ENCODER_LEFT_PIN, GPIO_IRQ_EDGE_FALL, true, &irq_handler);
}

void irq_handler(uint gpio, uint32_t events)
{
    if (gpio == BTN_DECREASE_SPEED)
    {
        duty_cycle -= 0.25;
        if (duty_cycle < 0.f)
            duty_cycle = 0.0f;
        set_wheels_duty_cycle(duty_cycle);
    }
    else if (gpio == BTN_MOVE)
    {
        if (isCarMoving)
        {
            set_car_state(CAR_STATIONARY);
            isCarMoving = false;
        }
        else 
        {
            set_car_state(CAR_FORWARD);
            isCarMoving = true;
        }
    }
    else if (gpio == BTN_INCREASE_SPEED)
    {
        duty_cycle += 0.25;
        if (duty_cycle > 1.f)
            duty_cycle = 1.f;
        set_wheels_duty_cycle(duty_cycle);
    }
    else if (gpio == WHEEL_ENCODER_LEFT_PIN)
    {
        printf("Wheel Encoder Left\n");
    }
    else if (gpio == WHEEL_ENCODER_RIGHT_PIN)
    {
        // printf("Wheel Encoder right");
    }
}