#include "encoder.h"

int main() {
    stdio_init_all();
    setupEncoderPins();

    // Set up interrupts for both encoder pins with a single callback
    gpio_set_irq_enabled_with_callback(WHEEL_ENCODER_LEFT_PIN, GPIO_IRQ_EDGE_FALL, true, &encoderCallback);
    gpio_set_irq_enabled_with_callback(WHEEL_ENCODER_RIGHT_PIN, GPIO_IRQ_EDGE_FALL, true, &encoderCallback);

    uint64_t last_print_time = time_us_64();
    const uint64_t print_interval_us = 1000000;

    while (1) {
        uint64_t current_time = time_us_64();

        // Check if the car has stopped for each wheel
        // checkIfStopped();

        // Print encoder data every second
        if (current_time - last_print_time >= print_interval_us) {
            // printEncoderData();
            last_print_time = current_time;
        }
    }

    return 0;
}
