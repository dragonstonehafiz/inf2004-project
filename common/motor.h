#ifndef MOTOR_H_
#define MOTOR_H_

#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include <stdio.h>

void setup_motor(uint pwm_pin, uint out_pin_1, uint out_pin_2);
void setup_motor_pwm(uint pwm_pin, float duty_cycle);
void set_motor_pwm_duty_cycle(uint pwm_pin, float duty_cycle);
void set_motor_direction(uint out_pin_1, uint out_pin_2, bool clockwise);
void set_motor_stop(uint out_pin_1, uint out_pin_2);

#endif