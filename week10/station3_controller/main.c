#include "udp.h"
#include "accelerometer.h"

int main() {
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

    int retry_count = 0;
    while (retry_count < MAX_WIFI_RETRIES) {
        printf("Attempting to connect to WiFi... (%d/%d)\n", retry_count + 1, MAX_WIFI_RETRIES);
        
        if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, 
            CYW43_AUTH_WPA2_AES_PSK, WIFI_CONNECT_TIMEOUT_MS) == 0) {
            printf("Wi-Fi connected successfully.\n");
            printf("IP Address: %s\n", ip4addr_ntoa(netif_ip4_addr(netif_list)));
            run_udp_beacon();  // Start beacon if connected
            break;
        }
        
        printf("Wi-Fi connection attempt %d failed.\n", retry_count + 1);
        retry_count++;
        sleep_ms(10); // Wait before retrying
    }

    if (retry_count == MAX_WIFI_RETRIES) {
        printf("Failed to connect to Wi-Fi after %d attempts. Exiting.\n", MAX_WIFI_RETRIES);
    }

    cyw43_arch_deinit();
    return 0;

}