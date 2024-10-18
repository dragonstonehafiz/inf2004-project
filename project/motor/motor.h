#ifndef MOTOR_H_
#define MOTOR_H_

#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include <stdio.h>

#define WHEEL_RIGHT_PWN_PIN 2
#define WHEEL_RIGHT_OUT_PIN_1 0
#define WHEEL_RIGHT_OUT_PIN_2 1

#define WHEEL_LEFT_PWN_PIN 8
#define WHEEL_LEFT_OUT_PIN_1 6
#define WHEEL_LEFT_OUT_PIN_2 7

#define CLOCK_FREQ 125000000
#define PWM_WRAP 65535
#define PWM_FREQ 25
#define PWM_DIVIDER ((float)CLOCK_FREQ / (PWM_WRAP * PWM_FREQ))

// true = clockwise
bool left_wheel_dir = true;
bool right_wheel_dir = false;

void init_motor();
void setup_pwm(uint pwm_pin, float duty_cycle);
void set_pwm_duty_cycle(uint pwm_pin, float duty_cycle);

void init_motor()
{
    setup_pwm(WHEEL_RIGHT_PWN_PIN, 0.f);
    gpio_init(WHEEL_RIGHT_OUT_PIN_1);
    gpio_init(WHEEL_RIGHT_OUT_PIN_2);
    gpio_set_dir(WHEEL_RIGHT_OUT_PIN_1, GPIO_OUT);
    gpio_set_dir(WHEEL_RIGHT_OUT_PIN_2, GPIO_OUT);
    gpio_put(WHEEL_RIGHT_OUT_PIN_1, right_wheel_dir);
    gpio_put(WHEEL_RIGHT_OUT_PIN_2, !right_wheel_dir);

    setup_pwm(WHEEL_LEFT_PWN_PIN, 0.f);
    gpio_init(WHEEL_LEFT_OUT_PIN_1);
    gpio_init(WHEEL_LEFT_OUT_PIN_2);
    gpio_set_dir(WHEEL_LEFT_OUT_PIN_1, GPIO_OUT);
    gpio_set_dir(WHEEL_LEFT_OUT_PIN_2, GPIO_OUT);
    gpio_put(WHEEL_LEFT_OUT_PIN_1, left_wheel_dir);
    gpio_put(WHEEL_LEFT_OUT_PIN_2, !left_wheel_dir);
}
void setup_pwm(uint pwm_pin, float duty_cycle) 
{
    // Set the GPIO function to PWM
    gpio_set_function(pwm_pin, GPIO_FUNC_PWM);

    // Find out which PWM slice is connected to the specified GPIO
    uint slice_num = pwm_gpio_to_slice_num(pwm_pin);
    pwm_set_clkdiv(slice_num, PWM_DIVIDER);
    pwm_set_wrap(slice_num, PWM_WRAP);
    pwm_set_gpio_level(pwm_pin, (uint16_t)(duty_cycle * (PWM_WRAP + 1)));
    set_pwm_duty_cycle(pwm_pin, duty_cycle);

    pwm_set_enabled(slice_num, true);
}
void set_pwm_duty_cycle(uint pwm_pin, float duty_cycle) 
{
    pwm_set_gpio_level(pwm_pin, (uint16_t)(duty_cycle * (PWM_WRAP + 1)));
}


#endif