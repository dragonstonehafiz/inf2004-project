#include "lwip/pbuf.h"
#include "lwip/udp.h"

typedef struct {
    char device_name[32];
    char forward_direction;
    float forward_percentage;
    char turn_direction;
    float turn_percentage;
} movement_data_t;

movement_data_t *get_movement_data(void);
void print_movement_data(movement_data_t * movement_data);

int init_server();
int deinit_server();
int connect_to_wifi_car();
