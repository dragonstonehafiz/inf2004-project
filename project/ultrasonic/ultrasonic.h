#ifndef ULTRASONIC_H
#define ULTRASONIC_H

#include "pico/stdlib.h"
#include <stdint.h> 
#include <stdio.h>
#include "hardware/gpio.h"
#include "hardware/timer.h"

// Pin definitions
#define BUZZER_PIN 18
#define TRIG_PIN 2
#define ECHO_PIN 3

// Distance threshold for the buzzer activation in centimeters
#define DISTANCE_THRESHOLD_CM 10.0

// Interval in milliseconds for checking the distance
#define CHECK_INTERVAL_MS 200

// Function declarations
void setupUltrasonicPins(unsigned int trigPin, unsigned int echoPin)
{
    gpio_init(trigPin);
    gpio_init(echoPin);
    gpio_set_dir(trigPin, GPIO_OUT);  
    gpio_set_dir(echoPin, GPIO_IN);   
}

void setupBuzzerPin()
{
    gpio_init(BUZZER_PIN);
    gpio_set_dir(BUZZER_PIN, GPIO_OUT);  
}

uint64_t getPulse(unsigned int trigPin, unsigned int echoPin)
{
    gpio_put(trigPin, 0);  
    sleep_us(2);
    gpio_put(trigPin, 1);  
    sleep_us(10);          
    gpio_put(trigPin, 0);  

    uint64_t startWait = time_us_64();
    while (gpio_get(echoPin) == 0) {
        if (time_us_64() - startWait > 30000) {
            return 0;  
        }
    }

    uint64_t pulseStart = time_us_64();
    while (gpio_get(echoPin) == 1) {
        if (time_us_64() - pulseStart > 30000) {
            return 0;  
        }
    }
    uint64_t pulseEnd = time_us_64();

    return pulseEnd - pulseStart;
}

float getCm(unsigned int trigPin, unsigned int echoPin)
{
    uint64_t pulseLength = getPulse(trigPin, echoPin);
    if (pulseLength == 0) return -1.0;  
    return (float)pulseLength / 29.0 / 2.0; 
}

#endif 