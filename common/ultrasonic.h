#ifndef ULTRASONIC_H
#define ULTRASONIC_H

#include "pico/stdlib.h"
#include <stdint.h> 
#include <stdio.h>
#include "hardware/gpio.h"
#include "hardware/timer.h"
#include "pins.h"

// Interval in milliseconds for checking the distance
#define CHECK_INTERVAL_MS 200

// Function declarations
void setupUltrasonicPins()
{
    gpio_init(TRIG_PIN);
    gpio_init(ECHO_PIN);
    gpio_set_dir(TRIG_PIN, GPIO_OUT);  
    gpio_set_dir(ECHO_PIN, GPIO_IN);   
}

void setupBuzzerPin()
{
    gpio_init(BUZZER_PIN);
    gpio_set_dir(BUZZER_PIN, GPIO_OUT);  
}

uint64_t getPulse()
{
    gpio_put(TRIG_PIN, 0);  
    sleep_us(2);
    gpio_put(TRIG_PIN, 1);  
    sleep_us(10);          
    gpio_put(TRIG_PIN, 0);  

    uint64_t startWait = time_us_64();
    while (gpio_get(ECHO_PIN) == 0) {
        if (time_us_64() - startWait > 30000) {
            return 0;  
        }
    }

    uint64_t pulseStart = time_us_64();
    while (gpio_get(ECHO_PIN) == 1) {
        if (time_us_64() - pulseStart > 30000) {
            return 0;  
        }
    }
    uint64_t pulseEnd = time_us_64();

    return pulseEnd - pulseStart;
}

float getCm()
{
    uint64_t pulseLength = getPulse(TRIG_PIN, ECHO_PIN);
    if (pulseLength == 0) return -1.0;  
    return (float)pulseLength / 29.0 / 2.0; 
}

#endif 