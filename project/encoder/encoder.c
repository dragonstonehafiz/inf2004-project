#include "encoder.h"

int main() {
    stdio_init_all();
    setupEncoderPins();

    uint64_t last_print_time = time_us_64();
    const uint64_t print_interval_us = 1000000;

    while (1) {
        uint64_t current_time = time_us_64();

        // Check if the car has stopped for each wheel
        checkIfStopped();

        // Print encoder data every second
        if (current_time - last_print_time >= print_interval_us) {
            printEncoderData();
            last_print_time = current_time;
        }
    }

    return 0;
}
