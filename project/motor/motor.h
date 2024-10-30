#ifndef MOTOR_H_
#define MOTOR_H_

#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include <stdio.h>

#define WHEEL_LEFT_PWN_PIN 10
#define WHEEL_LEFT_OUT_PIN_1 8
#define WHEEL_LEFT_OUT_PIN_2 9

#define WHEEL_RIGHT_PWN_PIN 11
#define WHEEL_RIGHT_OUT_PIN_1 2
#define WHEEL_RIGHT_OUT_PIN_2 3

#define CLOCK_FREQ 125000000
#define PWM_WRAP 65535
#define PWM_FREQ 25
#define PWM_DIVIDER ((float)CLOCK_FREQ / (PWM_WRAP * PWM_FREQ))

// true = clockwise
bool left_wheel_dir = true;
bool right_wheel_dir = true;

// PID Variables
float Kp = 0.1, Ki = 0.01, Kd = 0.005;

void init_motor();
void setup_pwm(uint pwm_pin, float duty_cycle);
void set_pwm_duty_cycle(uint pwm_pin, float duty_cycle);
void compute_pid(float target_speed, float current_speed, float *duty_cycle, float *integral, float *prev_error);

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
void compute_pid(float target_speed, float current_speed, float *duty_cycle, float *integral, float *prev_error)
{
    float error = target_speed - current_speed;
    *integral += error;
    float derivative = error - *prev_error;

    *duty_cycle += Kp * error + Ki * (*integral) + Kd * derivative;

    // Clamp the duty cycle to the range [0, 1]
    if (*duty_cycle > 1.0) *duty_cycle = 1.0;
    else if (*duty_cycle < 0) *duty_cycle = 0;

    *prev_error = error;
}


#endif