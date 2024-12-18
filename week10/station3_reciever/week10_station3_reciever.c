#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "lwip/pbuf.h"
#include "lwip/udp.h"

#define UDP_PORT 4444
#define MAX_WIFI_RETRIES 3
#define WIFI_CONNECT_TIMEOUT_MS 10000

// Make sure these are defined in CMakeLists.txt
#ifndef WIFI_SSID
#error "Please define WIFI_SSID in CMakeLists.txt"
#endif

#ifndef WIFI_PASSWORD
#error "Please define WIFI_PASSWORD in CMakeLists.txt"
#endif

void udp_receive_callback(void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port) {
    if (p != NULL) {
        // Create a buffer for the received data
        char* received_data = (char*)malloc(p->len + 1);
        memcpy(received_data, p->payload, p->len);
        received_data[p->len] = '\0';  // Null terminate the string

        // Variables to store parsed data  
        char device_name[32];
        char movement_dir;
        char turn_dir;
        float forward_speed;
        float turn_speed;
        
        // Parse the device name and data
        char* data_part = strchr(received_data, '|');
        if (data_part != NULL) {
            // Extract device name
            int device_name_len = data_part - received_data;
            strncpy(device_name, received_data, device_name_len);
            device_name[device_name_len] = '\0';
            
            // Move pointer past the '|'
            data_part++;
            
            // Parse the movement and turn data
            if (sscanf(data_part, "%c:%f,%c:%f", 
                   &movement_dir, &forward_speed, 
                   &turn_dir, &turn_speed) == 4) {
                
                printf("Device: %s, Movement: %c (%.1f%%), Turn: %c (%.1f%%)\n",
                       device_name, movement_dir, forward_speed, 
                       turn_dir, turn_speed);
            }
        }

        free(received_data);
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
        
        if (cyw43_arch_wifi_connect_timeout_ms("bighowdy", "yeedyourlasthaw", 
            CYW43_AUTH_WPA2_AES_PSK, WIFI_CONNECT_TIMEOUT_MS) == 0) {
            printf("WiFi connected successfully!\n");
            printf("IP Address: %s\n", ip4addr_ntoa(netif_ip4_addr(netif_list)));
            
            // Initialize UDP server after WiFi connection
            init_udp_server();
            
            // Keep the program running
            while(true) {
                // The callback will handle incoming packets
                sleep_ms(10);
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