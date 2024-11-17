#include <stdio.h>
#include "pico/stdlib.h"
#include "functions.h"

int main() {
    stdio_init_all();       // Initialize USB serial output
    printf("Testing Pico Example\n");

    // Initialize system components
    init_gpio();            // Initialize LED and button
    init_pwm();             // Set up PWM on LED
    start_timer();          // Set up and start repeating timer

    while (1) {
        //Calls the starting function
        main_fn();

        // Delay to simulate workload in the main loop
        sleep_ms(2000);
    }
}
