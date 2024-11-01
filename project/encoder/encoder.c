#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "include/hardware/encoder.h"

// Global variables to store measurement data for the left wheel
volatile uint32_t leftNotchCount = 0;
volatile uint32_t tempLeftNotchCount = 0;
volatile double leftTotalDistance = 0.0;
volatile double tempLeftTotalDistance = 0.0;
volatile uint64_t leftLastNotchTime = 0;
volatile double leftEncoderSpeed = 0.0;

// Global variables to store measurement data for the right wheel
volatile uint32_t rightNotchCount = 0;
volatile uint32_t tempRightNotchCount = 0;
volatile double rightTotalDistance = 0.0;
volatile double tempRightTotalDistance = 0.0;
volatile uint64_t rightLastNotchTime = 0;
volatile double rightEncoderSpeed = 0.0;

// Function to print current encoder data for both wheels
void printEncoderData(void) {
    printf("Left Wheel - Notch Count: %u, Distance: %.3f cm, Speed: %.4f cm/s\n",
           leftNotchCount, leftTotalDistance, leftEncoderSpeed);
    printf("Right Wheel - Notch Count: %u, Distance: %.3f cm, Speed: %.4f cm/s\n",
           rightNotchCount, rightTotalDistance, rightEncoderSpeed);
}

// Combined encoder callback to handle both left and right encoder interrupts
void encoderCallback(uint gpio, uint32_t events) {
    if (gpio == LEFT_ENCODER_PIN) {
        // Increment the count of notches detected for the left wheel
        leftNotchCount++;
        tempLeftNotchCount++;

        // Calculate the total distance traveled by the left wheel in centimeters
        leftTotalDistance = (double)leftNotchCount * CM_PER_NOTCH;
        tempLeftTotalDistance = (double)tempLeftNotchCount * CM_PER_NOTCH;

        // Calculate speed traveled after every cycle
        if (tempLeftNotchCount % NOTCHES_PER_CYCLE == 0) {
            // Get the current time in microseconds
            uint64_t currentTime = time_us_64();

            // Calculate the time difference between the current and previous notch
            uint64_t timeDiff = currentTime - leftLastNotchTime;

            if (timeDiff > 0) {
                // Calculate speed (distance traveled in 1 second) for the left wheel
                leftEncoderSpeed = tempLeftTotalDistance * 1e6 / timeDiff;
            }

            // Update the last notch time for the left wheel
            leftLastNotchTime = currentTime;
            tempLeftNotchCount = 0;
        }
    } else if (gpio == RIGHT_ENCODER_PIN) {
        // Increment the count of notches detected for the right wheel
        rightNotchCount++;
        tempRightNotchCount++;

        // Calculate the total distance traveled by the right wheel in centimeters
        rightTotalDistance = (double)rightNotchCount * CM_PER_NOTCH;
        tempRightTotalDistance = (double)tempRightNotchCount * CM_PER_NOTCH;

        // Calculate speed traveled after every cycle
        if (tempRightNotchCount % NOTCHES_PER_CYCLE == 0) {
            // Get the current time in microseconds
            uint64_t currentTime = time_us_64();

            // Calculate the time difference between the current and previous notch
            uint64_t timeDiff = currentTime - rightLastNotchTime;

            if (timeDiff > 0) {
                // Calculate speed (distance traveled in 1 second) for the right wheel
                rightEncoderSpeed = tempRightTotalDistance * 1e6 / timeDiff;
            }

            // Update the last notch time for the right wheel
            rightLastNotchTime = currentTime;
            tempRightNotchCount = 0;
        }
    }
}

// Setup function for the encoder pins and interrupts
void setupEncoderPins() {
    gpio_init(LEFT_ENCODER_PIN);
    gpio_set_dir(LEFT_ENCODER_PIN, GPIO_IN);

    gpio_init(RIGHT_ENCODER_PIN);
    gpio_set_dir(RIGHT_ENCODER_PIN, GPIO_IN);

    // Set up interrupts for both encoder pins with a single callback
    gpio_set_irq_enabled_with_callback(LEFT_ENCODER_PIN, GPIO_IRQ_EDGE_RISE, true, &encoderCallback);
    gpio_set_irq_enabled_with_callback(RIGHT_ENCODER_PIN, GPIO_IRQ_EDGE_RISE, true, &encoderCallback);
}

int main() {
    stdio_init_all(); 

    setupEncoderPins(); 

    uint64_t last_print_time = time_us_64();
    const uint64_t print_interval_us = 1000000;

    while (1) {
        // Print encoder data every second
        uint64_t current_time = time_us_64();
        if (current_time - last_print_time >= print_interval_us) {
            printEncoderData();
            last_print_time = current_time;
        }
    }

    return 0;
}
