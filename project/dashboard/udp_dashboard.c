#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "lwip/pbuf.h"
#include "lwip/udp.h"

// Define Wi-Fi credentials directly in the code
char wifi_ssid[] = "bighowdy";
char wifi_pwd[] = "yeedyourlasthaw";

#define UDP_PORT 4445
#define MAX_WIFI_RETRIES 3
#define WIFI_CONNECT_TIMEOUT_MS 10000

// Global variable to see if new data received
static bool new_data_received = false;

void udp_receive_callback(void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port) {
    if (p != NULL) {
        // Allocate memory for received data and null-terminate
        char* received_data = (char*)malloc(p->len + 1);
        if (received_data != NULL) {
            memcpy(received_data, p->payload, p->len);
            received_data[p->len] = '\0';

            // Indicate new data received and print it
            new_data_received = true;
            printf("Received data: %s\n", received_data);

            // Free allocated memory
            free(received_data);
        } else {
            printf("Memory allocation failed for received data\n");
        }

        // Free the pbuf
        pbuf_free(p);
    }
}

void init_udp_server() {
    // Create new UDP PCB
    struct udp_pcb* pcb = udp_new();
    if (pcb == NULL) {
        printf("Failed to create PCB\n");
        return;
    }

    // Bind to UDP port
    err_t err = udp_bind(pcb, IP_ADDR_ANY, UDP_PORT);
    if (err != ERR_OK) {
        printf("Failed to bind to port %d\n", UDP_PORT);
        return;
    }

    // Set receive callback
    udp_recv(pcb, udp_receive_callback, NULL);
    printf("UDP server listening on port %d\n", UDP_PORT);
}

int main() {
    stdio_init_all();
    sleep_ms(2000);
    printf("UDP Server Starting...\n");

    // Initialize WiFi
    if (cyw43_arch_init()) {
        printf("Failed to initialize WiFi\n");
        return -1;
    }

    printf("Connecting to Wi-Fi\n");

    cyw43_arch_enable_sta_mode();

    // Connect to WiFi
    int retry_count = 0;
    while (retry_count < MAX_WIFI_RETRIES) {
        printf("Attempting to connect to WiFi... (%d/%d)\n", retry_count + 1, MAX_WIFI_RETRIES);
        
        if (cyw43_arch_wifi_connect_timeout_ms(wifi_ssid, wifi_pwd, 
            CYW43_AUTH_WPA2_AES_PSK, WIFI_CONNECT_TIMEOUT_MS) == 0) {
            printf("WiFi connected successfully!\n");
            printf("IP Address: %s\n", ip4addr_ntoa(netif_ip4_addr(netif_list)));
            
            // Initialize UDP server after WiFi connection
            init_udp_server();

            // Keep the program running
            while (true) {
                if (new_data_received) {
                    new_data_received = false;
                    sleep_ms(10);
                }
            }
            break;
        }
        
        printf("WiFi connection attempt %d failed.\n", retry_count + 1);
        retry_count++;
        sleep_ms(1000);
    }

    if (retry_count == MAX_WIFI_RETRIES) {
        printf("Failed to connect to WiFi after %d attempts\n", MAX_WIFI_RETRIES);
    }

    cyw43_arch_deinit();
    return 0;
}
