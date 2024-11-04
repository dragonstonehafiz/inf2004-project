#include <string.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "lwip/pbuf.h"
#include "lwip/udp.h"
#include "accelerometer.h"

#define UDP_PORT 4444
#define BEACON_MSG_LEN_MAX 127
#define BEACON_TARGET "172.20.10.3"
#define BEACON_INTERVAL_MS 10
#define WIFI_CONNECT_TIMEOUT_MS 10000
#define MAX_WIFI_RETRIES 3

void run_udp_beacon() {
    struct udp_pcb* pcb = udp_new();
    if (pcb == NULL) {
        printf("Failed to create UDP PCB\n");
        return;
    }

    ip_addr_t addr;
    if (!ipaddr_aton(BEACON_TARGET, &addr)) {
        printf("Invalid target IP address\n");
        udp_remove(pcb);
        return;
    }

    // Initialize GP20 as input with pull-up
    gpio_init(20);
    gpio_set_dir(20, GPIO_IN);
    gpio_pull_up(20);

    bool is_running = false;
    bool prev_button_state = true;  // true because of pull-up (not pressed)

    while (true) {
        // Check button state
        bool current_button_state = gpio_get(20);
        
        // Detect button press (transition from high to low due to pull-up)
        if (prev_button_state && !current_button_state) {
            is_running = !is_running;  // Toggle state
            
            // If we're stopping, send a stop command
            if (!is_running) {
                struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT, BEACON_MSG_LEN_MAX + 1, PBUF_RAM);
                if (p != NULL) {
                    char *req = (char *)p->payload;
                    memset(req, 0, BEACON_MSG_LEN_MAX + 1);
                    // Send a command with no movement (N = None for both directions)
                    snprintf(req, BEACON_MSG_LEN_MAX, "%lu,N,0.0,N,0.0\n", 
                            to_ms_since_boot(get_absolute_time()));
                    
                    udp_sendto(pcb, p, &addr, UDP_PORT);
                    printf("Sent stop command\n");
                    pbuf_free(p);
                }
            }
            
            printf("UDP sending is now %s\n", is_running ? "ON" : "OFF");
            sleep_ms(50);  // Simple debounce
        }
        prev_button_state = current_button_state;

        // Only send movement data if is_running is true
        if (is_running) {
            char* data_to_send = get_data_to_send();
            
            struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT, BEACON_MSG_LEN_MAX + 1, PBUF_RAM);
            if (p == NULL) {
                printf("Failed to allocate pbuf\n");
                sleep_ms(BEACON_INTERVAL_MS);
                continue;
            }

            char *req = (char *)p->payload;
            memset(req, 0, BEACON_MSG_LEN_MAX + 1);
            snprintf(req, BEACON_MSG_LEN_MAX, "%s", data_to_send);

            err_t err = udp_sendto(pcb, p, &addr, UDP_PORT);
            if (err != ERR_OK) {
                printf("Failed to send UDP packet: %d\n", err);
            } else {
                printf("Sent data: %s", data_to_send);
            }

            pbuf_free(p);
        }
        
        sleep_ms(BEACON_INTERVAL_MS);
    }

    udp_remove(pcb);
}