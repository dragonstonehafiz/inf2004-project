#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include "pico/stdlib.h"

// For GPIO
void initButton(uint gpio);
void initLED(uint gpio);
void init();
// I'm reusing code from my car project
void setupPWM(uint pwm_pin, float duty_cycle);
void setPWMDutyCycle(uint pwm_pin, float duty_cycle);

// Interrupts
void irq_func(uint gpio, uint32_t events);

// For Logic
bool addChar(char toAdd);
void setLED(uint gpio, char toPrint);
void startPlayback();
void mainloop();

// Timer
int64_t reeanbleButtons(alarm_id_t id, void *user_data);

#endif
