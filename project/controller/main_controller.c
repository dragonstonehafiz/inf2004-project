#include "accelerometer.h"
#include "udp.h"
#include "pins.h"

#define BUTTON_PIN 20  // Define the GPIO pin for the button

int main() {
    bool connected = false;
    bool is_running = false;  // Flag to track if we're sending data
    bool prev_button_state = true;  // true because of pull-up (not pressed)


    stdio_init_all();
   
    sleep_ms(3000);  // Delay to allow serial to be opened
    // Initialize accelerometer
    init_accelerometer();
    printf("Accelerometer initialized\n");

    // Initialize button GPIO with pull-up
    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_PIN);
    
    // Connect to wifi
    connected = connect_to_wifi();

    if (connected){
        if (initialize_udp() == NULL) {
            printf("UDP initialization failed\n");
            connected = false;
        } else {
            printf("UDP initialized successfully\n");
        }
    } else {
        printf("WiFi connection failed\n");
    }
    

    while(connected) {
        // Check button state
        bool current_button_state = gpio_get(BUTTON_PIN);
        
        // Detect button press (transition from high to low due to pull-up)
        if (prev_button_state && !current_button_state) {
            is_running = !is_running;  // Toggle the state
            
            if (is_running) {
                printf("Started sending data\n");
            } else {
                printf("Stopped sending data\n");
            }
            
            sleep_ms(50);  // Simple debounce
        }
        prev_button_state = current_button_state;

        // Only call get_data_to_send if is_running is true
        if (is_running) {
            get_data_to_send();  // This function will handle both getting and sending data
        }
        
        sleep_ms(BEACON_INTERVAL_MS);
    }

    cyw43_arch_deinit();
    return 0;
}