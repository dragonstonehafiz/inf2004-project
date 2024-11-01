#ifndef _ENCODER_H
#define _ENCODER_H

#include <stdint.h>
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"

// Definitions for encoder GPIO pins and constants for calculations
#define LEFT_ENCODER_PIN 0        
#define RIGHT_ENCODER_PIN 2        
#define CM_PER_NOTCH 1.005  
#define TIMEOUT_THRESHOLD 1500000   

static int leftStopCounter = 0;
static int rightStopCounter = 0;

// Global variables to store measurement data for the left wheel
volatile uint32_t leftNotchCount = 0;
volatile double leftTotalDistance = 0.0;
volatile uint64_t leftLastNotchTime = 0;
volatile double leftEncoderSpeed = 0.0;

// Global variables to store measurement data for the right wheel
volatile uint32_t rightNotchCount = 0;
volatile double rightTotalDistance = 0.0;
volatile uint64_t rightLastNotchTime = 0;
volatile double rightEncoderSpeed = 0.0;

// Function to print current encoder data for both wheels
static inline void printEncoderData(void) {
    printf("Left Wheel - Notch Count: %u, Distance: %.4f cm, Speed: %.4f cm/s\n",
           leftNotchCount, leftTotalDistance, leftEncoderSpeed);
    printf("Right Wheel - Notch Count: %u, Distance: %.4f cm, Speed: %.4f cm/s\n",
           rightNotchCount, rightTotalDistance, rightEncoderSpeed);
}

// Combined encoder callback to handle both left and right encoder interrupts
static inline void encoderCallback(uint gpio, uint32_t events) {
    uint64_t currentTime = time_us_64();

    if (gpio == LEFT_ENCODER_PIN) {
        // Increment the count of notches detected for the left wheel
        leftNotchCount++;
        leftTotalDistance = (double)leftNotchCount * CM_PER_NOTCH;
        
        // Calculate time difference and speed for the left wheel
        uint64_t timeDiff = currentTime - leftLastNotchTime;
        if (timeDiff > 0) {
            leftEncoderSpeed = CM_PER_NOTCH * 1e6 / timeDiff;
        }

        leftLastNotchTime = currentTime;
    } else if (gpio == RIGHT_ENCODER_PIN) {
        // Increment the count of notches detected for the right wheel
        rightNotchCount++;
        rightTotalDistance = (double)rightNotchCount * CM_PER_NOTCH;

        // Calculate time difference and speed for the right wheel
        uint64_t timeDiff = currentTime - rightLastNotchTime;
        if (timeDiff > 0) {
            rightEncoderSpeed = CM_PER_NOTCH * 1e6 / timeDiff;
        }

        rightLastNotchTime = currentTime;
    }
}

// Function to check if the car has stopped and set speed to zero if no movement
static inline void checkIfStopped() {
    uint64_t currentTime = time_us_64();

    if (currentTime - leftLastNotchTime > TIMEOUT_THRESHOLD) {
        leftStopCounter++;
    } else {
        leftStopCounter = 0;
    }

    if (currentTime - rightLastNotchTime > TIMEOUT_THRESHOLD) {
        rightStopCounter++;
    } else {
        rightStopCounter = 0;
    }

    // Only set speed to zero if the counter exceeds a threshold which is 3 checks in a row
    if (leftStopCounter >= 3) {
        leftEncoderSpeed = 0.0;
    }

    if (rightStopCounter >= 3) {
        rightEncoderSpeed = 0.0;
    }
}

// Setup function for the encoder pins and interrupts
static inline void setupEncoderPins() {
    gpio_init(LEFT_ENCODER_PIN);
    gpio_set_dir(LEFT_ENCODER_PIN, GPIO_IN);

    gpio_init(RIGHT_ENCODER_PIN);
    gpio_set_dir(RIGHT_ENCODER_PIN, GPIO_IN);

    // Set up interrupts for both encoder pins with a single callback
    gpio_set_irq_enabled_with_callback(LEFT_ENCODER_PIN, GPIO_IRQ_EDGE_RISE, true, &encoderCallback);
    gpio_set_irq_enabled_with_callback(RIGHT_ENCODER_PIN, GPIO_IRQ_EDGE_RISE, true, &encoderCallback);
}

#endif
