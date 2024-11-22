#include "reciever.h"
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "states.h"

#define UDP_PORT 4444
#define MAX_WIFI_RETRIES 3
#define WIFI_CONNECT_TIMEOUT_MS 10000

// Global variable to store movement data
static movement_data_t movement_data = {"unknown", 'N', 0.0f, 'N', 0.0f};

// Global variable to see if new data received
static bool new_data_received = false;

// Putting declarations here so outside classes don't see things they shouldn't use
void init_udp_server();
void udp_receive_callback(void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port);

movement_data_t *get_movement_data(void) 
{
    if (new_data_received)
    {
        new_data_received = false;
        return &movement_data;
    }
    else
        return NULL;
}
void print_movement_data(movement_data_t * movement_data)
{
    if (movement_data != NULL)
    {
        printf("PICO|%c:%.2f,%c:%.2f\n",
                movement_data->forward_direction, movement_data->forward_percentage, 
                movement_data->turn_direction, movement_data->turn_percentage);
    }
}

int init_server()
{
    // Initialize WiFi
    if (cyw43_arch_init()) 
    {
        printf("Failed to initialize WiFi\n");
        return 0;
    }

    cyw43_arch_enable_sta_mode();
    return 1;
}

int deinit_server()
{
    cyw43_arch_deinit();
    return 1;
}

int connect_to_wifi()
{
    // Connect to WiFi
    int retry_count = 0;
    bool success = false;
    while (retry_count < MAX_WIFI_RETRIES) 
    {
        printf("\nAttempting to connect to %s (%d/%d)\n", WIFI_SSID, retry_count + 1, MAX_WIFI_RETRIES);
        
        if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, 
            CYW43_AUTH_WPA2_AES_PSK, WIFI_CONNECT_TIMEOUT_MS) == 0) {
            printf("WiFi connected successfully!\n");
            printf("IP Address: %s\n", ip4addr_ntoa(netif_ip4_addr(netif_list)));
            
            // Initialize UDP server after WiFi connection
            init_udp_server();
            changeState(STATE_REMOTE);
            success = true;
            break;
        }
        
        printf("WiFi connection attempt %d failed.\n", retry_count + 1);
        retry_count++;
        sleep_ms(1000);
    }

    if (!success)
    {
        printf("Connection failed. Press button 21 to try again.\n");
        changeState(STATE_INITIAL);
    }
    return 1;
}

void udp_receive_callback(void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port) {
    if (p != NULL) {
        char* received_data = (char*)malloc(p->len + 1);
        memcpy(received_data, p->payload, p->len);
        received_data[p->len] = '\0';
        
        char* data_part = strchr(received_data, '|');
        if (data_part != NULL) {
            int device_name_len = data_part - received_data;
            strncpy(movement_data.device_name, received_data, device_name_len);
            movement_data.device_name[device_name_len] = '\0';
            
            data_part++;
            
            char f_dir, t_dir;
            float f_perc, t_perc;
            
            if (sscanf(data_part, "%c:%f,%c:%f", &f_dir, &f_perc, &t_dir, &t_perc) == 4) {
                // Only update global data if parsing was successful
                movement_data.forward_direction = f_dir;
                movement_data.forward_percentage = f_perc;
                movement_data.turn_direction = t_dir;
                movement_data.turn_percentage = t_perc;

                new_data_received = true;
                
                printf("Device: %s, Movement: %c (%.1f%%), Turn: %c (%.1f%%)\n",
                       movement_data.device_name, movement_data.forward_direction, 
                       movement_data.forward_percentage,
                       movement_data.turn_direction, 
                       movement_data.turn_percentage);
            } else {
                printf("Failed to parse movement data\n");
            }
        } else {
            printf("Invalid data format - missing '|' separator\n");
        }

        free(received_data);
        pbuf_free(p);
    }
}
void init_udp_server()
{
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
