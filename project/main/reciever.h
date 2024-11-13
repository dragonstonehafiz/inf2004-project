#include "lwip/pbuf.h"
#include "lwip/udp.h"

int init_server();
int deinit_server();
int connect_to_wifi();
void udp_receive_callback(void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port);
void init_udp_server();