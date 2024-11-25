#include "network.h"
#include "pins.h"

#define WIFI_CONNECT_TIMEOUT_MS 10000
#define MAX_WIFI_RETRIES 3

char wifi_ssid[] = "bighowdy";
char wifi_pwd[] = "yeedyourlasthaw";

// FOR UDP_SERVER
static struct udp_pcb *udp_pcb = NULL;

// Global variable to store movement data
static movement_data_t movement_data = {"unknown", 'N', 0.0f, 'N', 0.0f};
// Global variable to see if new data received
static bool new_data_received = false;

int init_server()
{
    gpio_init(WIFI_INDICATOR_PIN_1);
    gpio_set_dir(WIFI_INDICATOR_PIN_1, GPIO_OUT);
    gpio_init(WIFI_INDICATOR_PIN_2);
    gpio_set_dir(WIFI_INDICATOR_PIN_2, GPIO_OUT);

    // Initialize WiFi
    if (cyw43_arch_init())
    {
        printf("Failed to initialize WiFi\n");
        return 0;
    }

    printf("Wi-Fi initialized successfully\n");
    cyw43_arch_enable_sta_mode();
    return 1;
}
int deinit_server()
{
    cyw43_arch_deinit();
    return 1;
}
bool connect_to_wifi()
{
    printf("Connecting to Wi-Fi\n");

    int retry_count = 0;
    while (retry_count < MAX_WIFI_RETRIES)
    {
        // Enable first indicator pin to show connection is being attempted
        gpio_put(WIFI_INDICATOR_PIN_1, 1);
        printf("Attempting to connect to %s... (%d/%d)\n", wifi_ssid, retry_count + 1, MAX_WIFI_RETRIES);

        if (cyw43_arch_wifi_connect_timeout_ms(wifi_ssid, wifi_pwd,
                                               CYW43_AUTH_WPA2_AES_PSK, WIFI_CONNECT_TIMEOUT_MS) == 0)
        {
            printf("Wi-Fi connected successfully.\n");
            printf("IP Address: %s\n", ip4addr_ntoa(netif_ip4_addr(netif_list)));

            // Enable second indicator pin to show connection is successfull
            gpio_put(WIFI_INDICATOR_PIN_2, 1);
            return true;
        }

        printf("Wi-Fi connection attempt %d failed.\n", retry_count + 1);
        retry_count++;
        // Disable first indicator pin to show connection failed, then wait 1 second
        gpio_put(WIFI_INDICATOR_PIN_1, 0);
        sleep_ms(1000); // Wait before retrying
    }

    printf("Failed to connect to Wi-Fi after %d attempts.\n", MAX_WIFI_RETRIES);
    return false;
}

// FOR UDP_SERVER
void init_udp_server_reciever(uint16_t udp_port)
{
    // Create new UDP PCB
    struct udp_pcb *pcb = udp_new();
    if (pcb == NULL)
    {
        printf("Failed to create PCB\n");
        return;
    }

    // Bind to UDP port
    err_t err = udp_bind(pcb, IP_ADDR_ANY, udp_port);
    if (err != ERR_OK)
    {
        printf("Failed to bind to port %d\n", udp_port);
        return;
    }

    // Set receive callback
    udp_recv(pcb, udp_receive_callback, NULL);
    printf("UDP server listening on port %d\n", udp_port);
}
void udp_receive_callback(void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port)
{
    if (p != NULL)
    {
        char *received_data = (char *)malloc(p->len + 1);
        memcpy(received_data, p->payload, p->len);
        received_data[p->len] = '\0';

        char *data_part = strchr(received_data, '|');
        if (data_part != NULL)
        {
            int device_name_len = data_part - received_data;
            strncpy(movement_data.device_name, received_data, device_name_len);
            movement_data.device_name[device_name_len] = '\0';

            data_part++;

            char f_dir, t_dir;
            float f_perc, t_perc;

            if (sscanf(data_part, "%c:%f,%c:%f", &f_dir, &f_perc, &t_dir, &t_perc) == 4)
            {
                // Only update global data if parsing was successful
                movement_data.forward_direction = f_dir;
                movement_data.forward_percentage = f_perc;
                movement_data.turn_direction = t_dir;
                movement_data.turn_percentage = t_perc;

                new_data_received = true;
            }
            else
            {
                printf("Failed to parse movement data\n");
            }
        }
        else
        {
            printf("Invalid data format - missing '|' separator\n");
        }

        free(received_data);
        pbuf_free(p);
    }
}
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

// FOR UDP_CLIENT
struct udp_pcb *init_udp_server_sender(const char *IP)
{
    udp_pcb = udp_new();
    if (udp_pcb == NULL)
    {
        printf("Failed to create UDP PCB\n");
        return NULL;
    }

    ip_addr_t addr;
    if (!ipaddr_aton(IP, &addr))
    {
        printf("Invalid target IP address\n");
        udp_remove(udp_pcb);
        udp_pcb = NULL;
        return NULL;
    }

    return udp_pcb;
}
void send_udp_data(const char *data, uint16_t udp_port, const char *IP)
{
    int BEACON_MSG_LEN_MAX = 127;

    if (udp_pcb == NULL)
    {
        printf("UDP not initialized\n");
        return;
    }

    ip_addr_t addr;
    ipaddr_aton(IP, &addr);

    struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT, BEACON_MSG_LEN_MAX + 1, PBUF_RAM);
    if (p == NULL)
    {
        printf("Failed to allocate pbuf\n");
        return;
    }

    char *req = (char *)p->payload;
    memset(req, 0, BEACON_MSG_LEN_MAX + 1);
    snprintf(req, BEACON_MSG_LEN_MAX, "%s", data);

    err_t err = udp_sendto(udp_pcb, p, &addr, udp_port);
    if (err != ERR_OK)
        printf("Failed to send UDP packet: %d\n", err);
    // else
    //     printf("Sent data: %s\n", data);

    pbuf_free(p);
}
