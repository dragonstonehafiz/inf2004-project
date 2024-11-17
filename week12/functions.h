#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include "pico/stdlib.h"

// Function declarations
void init_gpio();
void button_callback(uint gpio, uint32_t events);
void init_pwm();
void start_timer();
int example_function(int a, int b);
void main_fn();

#endif
