#ifndef _ENCODER_H
#define _ENCODER_H

#include <stdint.h>
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "pins.h"

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

volatile float leftPulseWidth = 0.0;
volatile float rightPulseWidth = 0.0;

// Function to print current encoder data for both wheels
static inline void printEncoderData(void) {
    printf("Left Wheel - Notch Count: %u, Distance: %.4f cm, Speed: %.4f cm/s\n",
           leftNotchCount, leftTotalDistance, leftEncoderSpeed);
    printf("Right Wheel - Notch Count: %u, Distance: %.4f cm, Speed: %.4f cm/s\n",
           rightNotchCount, rightTotalDistance, rightEncoderSpeed);
}

// Combined encoder callback to handle both left and right encoder interrupts
void encoderCallback(uint gpio, uint32_t events) {
    uint64_t currentTime = time_us_64();

    if (gpio == WHEEL_ENCODER_LEFT_PIN) {
        // Calculate time difference and speed for the left wheel
        uint64_t timeDiff = currentTime - leftLastNotchTime;
        if (timeDiff > 0 && timeDiff < TIMEOUT_THRESHOLD) {           
            // Increment the count of notches detected for the left wheel
            leftNotchCount++;
            leftTotalDistance = (double)leftNotchCount * CM_PER_NOTCH;
            leftEncoderSpeed = CM_PER_NOTCH / (timeDiff / 1e6);
            leftPulseWidth = timeDiff / 1e6;
        } else {
            leftEncoderSpeed = 0.0;
        }

        leftLastNotchTime = currentTime;
    } else if (gpio == WHEEL_ENCODER_RIGHT_PIN) {

        // Calculate time difference and speed for the right wheel
        // If time diff is larger than acceptable threshold, assume right encoder was stopped
        uint64_t timeDiff = currentTime - rightLastNotchTime;
        if (timeDiff > 0  && timeDiff < TIMEOUT_THRESHOLD) {     
            // Increment the count of notches detected for the right wheel
            rightNotchCount++;
            rightTotalDistance = (double)rightNotchCount * CM_PER_NOTCH;
            rightEncoderSpeed = CM_PER_NOTCH / (timeDiff / 1e6);
            rightPulseWidth = timeDiff / 1e6;
        } else {
            rightEncoderSpeed = 0.0;
        }

        rightLastNotchTime = currentTime;
    }
}

// Function to check if the car has stopped and set speed to zero if no movement
void checkIfStopped() {
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
void setupEncoderPins() {
    gpio_init(WHEEL_ENCODER_LEFT_PIN);
    gpio_set_dir(WHEEL_ENCODER_LEFT_PIN, GPIO_IN);

    gpio_init(WHEEL_ENCODER_RIGHT_PIN);
    gpio_set_dir(WHEEL_ENCODER_RIGHT_PIN, GPIO_IN);
}

#endif
