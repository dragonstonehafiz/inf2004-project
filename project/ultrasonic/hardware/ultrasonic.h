// ultrasonic.h

#ifndef ULTRASONIC_H
#define ULTRASONIC_H

#include "pico/stdlib.h"
#include <stdint.h>  // Include this to define uint64_t

// Pin definitions
#define BUZZER_PIN 18
#define TRIG_PIN 2
#define ECHO_PIN 3

// Distance threshold for the buzzer activation in centimeters
#define DISTANCE_THRESHOLD_CM 10.0

// Interval in milliseconds for checking the distance
#define CHECK_INTERVAL_MS 200

// Function declarations
void setupUltrasonicPins(unsigned int trigPin, unsigned int echoPin);
void setupBuzzerPin();
uint64_t getPulse(unsigned int trigPin, unsigned int echoPin);
float getCm(unsigned int trigPin, unsigned int echoPin);

#endif 