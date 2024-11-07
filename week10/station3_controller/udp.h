#ifndef UDP_H
#define UDP_H

#include <string.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "lwip/pbuf.h"
#include "lwip/udp.h"

// Define constants
#define UDP_PORT 4444
#define BEACON_MSG_LEN_MAX 127
#define BEACON_TARGET "172.20.10.5"
#define BEACON_INTERVAL_MS 10
#define WIFI_CONNECT_TIMEOUT_MS 10000
#define MAX_WIFI_RETRIES 3

// Global variable for UDP PCB
static struct udp_pcb* udp_pcb = NULL;

// Function declarations
struct udp_pcb* initialize_udp(void);
void send_udp_data(const char* data);

// Function implementations
struct udp_pcb* initialize_udp() {
    udp_pcb = udp_new();
    if (udp_pcb == NULL) {
        printf("Failed to create UDP PCB\n");
        return NULL;
    }

    ip_addr_t addr;
    if (!ipaddr_aton(BEACON_TARGET, &addr)) {
        printf("Invalid target IP address\n");
        udp_remove(udp_pcb);
        udp_pcb = NULL;
        return NULL;
    }

    printf("UDP initialized successfully\n");
    return udp_pcb;
}

void send_udp_data(const char* data) {
    if (udp_pcb == NULL) {
        printf("UDP not initialized\n");
        return;
    }

    ip_addr_t addr;
    ipaddr_aton(BEACON_TARGET, &addr);

    struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT, BEACON_MSG_LEN_MAX + 1, PBUF_RAM);
    if (p == NULL) {
        printf("Failed to allocate pbuf\n");
        return;
    }

    char *req = (char *)p->payload;
    memset(req, 0, BEACON_MSG_LEN_MAX + 1);
    snprintf(req, BEACON_MSG_LEN_MAX, "%s", data);

    err_t err = udp_sendto(udp_pcb, p, &addr, UDP_PORT);
    if (err != ERR_OK) {
        printf("Failed to send UDP packet: %d\n", err);
    } else {
        printf("Sent data: %s", data);
    }

    pbuf_free(p);
}

#endif // UDP_H