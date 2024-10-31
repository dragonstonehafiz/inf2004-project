#ifndef MOTOR_H_
#define MOTOR_H_

#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include <stdio.h>
#include "pins.h"

#define PWM_WRAP 65535

void init_motor();
void init_left_motor();
void init_right_motor();

void setup_pwm(uint pwm_pin, float duty_cycle);
void set_pwm_duty_cycle(uint pwm_pin, float *duty_cycle);

void compute_wheel_duty_cycle(float *target_speed, float *current_speed, float *duty_cycle, float *integral, float *prev_error);

void turn_wheel_clockwise(uint pwm_pin);
void turn_wheel_anticlockwise(uint pwm_pin);
void set_wheel_duty_cycle(uint pwm_pin, float *dutyCycle);

#endif