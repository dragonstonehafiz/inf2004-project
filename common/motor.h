#ifndef MOTOR_H_
#define MOTOR_H_

#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include <stdio.h>

#define PWM_WRAP 65535

void setup_motor(uint pwm_pin, uint out_pin_1, uint out_pin_2);
void setup_motor_pwm(uint pwm_pin, float duty_cycle);
void set_motor_pwm_duty_cycle(uint pwm_pin, float duty_cycle);
void set_motor_direction(uint out_pin_1, uint out_pin_2, bool clockwise);
void set_motor_stop(uint out_pin_1, uint out_pin_2);

void setup_motor(uint pwm_pin, uint out_pin_1, uint out_pin_2)
{
    setup_motor_pwm(pwm_pin, 0.f);
    gpio_init(out_pin_1);
    gpio_init(out_pin_2);
    gpio_set_dir(out_pin_1, GPIO_OUT);
    gpio_set_dir(out_pin_2, GPIO_OUT);
}
void setup_motor_pwm(uint pwm_pin, float duty_cycle) 
{
    // Calculate the PWM frequency and set the PWM wrap value
    float clock_freq = 125000000.0f;  // Default Pico clock frequency in Hz
    uint16_t freq = 25;
    uint32_t divider = clock_freq / (freq * PWM_WRAP);  // Compute divider for given frequency

    // Set the GPIO function to PWM
    gpio_set_function(pwm_pin, GPIO_FUNC_PWM);

    // Find out which PWM slice is connected to the specified GPIO
    uint slice_num = pwm_gpio_to_slice_num(pwm_pin);
    pwm_set_clkdiv(slice_num, divider);
    pwm_set_wrap(slice_num, PWM_WRAP);
    pwm_set_gpio_level(pwm_pin, (uint16_t)(duty_cycle * (PWM_WRAP + 1)));
    set_motor_pwm_duty_cycle(pwm_pin, duty_cycle);

    pwm_set_enabled(slice_num, true);
}
void set_motor_pwm_duty_cycle(uint pwm_pin, float duty_cycle) 
{
    pwm_set_gpio_level(pwm_pin, (uint16_t)((duty_cycle) * (PWM_WRAP - 1)));
}
void set_motor_direction(uint out_pin_1, uint out_pin_2, bool clockwise)
{
    gpio_put(out_pin_1, clockwise);
    gpio_put(out_pin_2, !clockwise);
}
void set_motor_stop(uint out_pin_1, uint out_pin_2)
{
    gpio_put(out_pin_1, true);
    gpio_put(out_pin_2, true);
}

#endif