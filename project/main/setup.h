#include <stdio.h>


void init_gpio();
void init_interrupts();
void irq_handler(uint gpio, uint32_t events);

