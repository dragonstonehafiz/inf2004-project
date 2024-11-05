#include "accelerometer.h"

#define BUTTON_PIN 20  // Define the GPIO pin for the button

int main() {
    bool connected = false;
    bool is_running = false;  // Flag to track if we're sending data
    bool prev_button_state = true;  // true because of pull-up (not pressed)

    stdio_init_all();

    printf("Connecting to Wi-Fi\n");

    // Initialize Wi-Fi
    if (cyw43_arch_init()) {
        printf("Failed to initialize Wi-Fi.\n");
        return -1;
    }

    // Enable station mode
    cyw43_arch_enable_sta_mode();
   
    // Initialize accelerometer
    init_accelerometer();
    printf("Accelerometer initialized\n");

    // Initialize button GPIO with pull-up
    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_PIN);

    int retry_count = 0;
    while (retry_count < MAX_WIFI_RETRIES) {
        printf("Attempting to connect to WiFi... (%d/%d)\n", retry_count + 1, MAX_WIFI_RETRIES);
        
        if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, 
            CYW43_AUTH_WPA2_AES_PSK, WIFI_CONNECT_TIMEOUT_MS) == 0) {
            printf("Wi-Fi connected successfully.\n");
            printf("IP Address: %s\n", ip4addr_ntoa(netif_ip4_addr(netif_list)));
            initialize_udp();
            connected = true;
            break;
        }
        
        printf("Wi-Fi connection attempt %d failed.\n", retry_count + 1);
        retry_count++;
        sleep_ms(10); // Wait before retrying
    }

    if (retry_count == MAX_WIFI_RETRIES) {
        printf("Failed to connect to Wi-Fi after %d attempts. Exiting.\n", MAX_WIFI_RETRIES);
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