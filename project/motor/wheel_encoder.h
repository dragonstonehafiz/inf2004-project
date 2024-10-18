#ifndef WHEEL_ENCODER_H_
#define WHEEL_ENCODER_H_

#include "pico/stdlib.h"
#include <stdio.h>

#define WHEEL_ENCODER_RIGHT_PIN 3
#define WHEEL_ENCODER_NUM_SLITS 20

uint64_t last_time_r = 0;
uint64_t last_time_l = 0;

void init_wheel_encoder();
void irq_ir_sensor(uint gpio);

void init_wheel_encoder()
{
    gpio_init(WHEEL_ENCODER_RIGHT_PIN);
    gpio_set_dir(WHEEL_ENCODER_RIGHT_PIN, GPIO_IN);
    gpio_pull_up(WHEEL_ENCODER_RIGHT_PIN);
}
void irq_ir_sensor(uint gpio)
{
    if (gpio == WHEEL_ENCODER_RIGHT_PIN)
    {
        // Gets the current time in micro seconds (1e-6)
        uint64_t current_time = time_us_64();
        float pulse_width = 0;
        // This is just so we don't calculate pulse width at the start of the program
        if (last_time_r != 0)
            pulse_width = (current_time - last_time_r) / 1000000.f;
        last_time_r = current_time;
        printf("Pulse Width Right: %0.4fs\n", pulse_width);
    }
}


#endif