#ifndef NETWORK_H_
#define NETWORK_H_

#include <string.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "lwip/pbuf.h"
#include "lwip/udp.h"
#include "accelerometer.h"

#define IP_CAR "192.168.18.90"
#define IP_DASHBOARD "192.168.18.225"
#define PORT_CAR 4444
#define PORT_DASHBOARD 4445

extern char wifi_ssid[];
extern char wifi_pwd[];

typedef struct
{
    char device_name[32];
    char forward_direction;
    float forward_percentage;
    char turn_direction;
    float turn_percentage;
} movement_data_t;

// GENERAL
int init_server();
int deinit_server();
bool connect_to_wifi();

// FOR UDP SERVER
void init_udp_server_reciever(uint16_t udp_port);
void udp_receive_callback(void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port);
movement_data_t *get_movement_data(void);

// FOR UDP CLIENT
struct udp_pcb *init_udp_server_sender(const char *IP);
void send_udp_data(const char *data, uint16_t udp_port, const char *IP);

#endif