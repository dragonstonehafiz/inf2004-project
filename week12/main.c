#include <stdio.h>
#include "pico/stdlib.h"
#include "functions.h"

int main() {
    stdio_init_all();       // Initialize USB serial output
    printf("Testing Pico Example\n");

    // Initialize system components
    init();

    while (1) {
        mainloop();
    }
}
