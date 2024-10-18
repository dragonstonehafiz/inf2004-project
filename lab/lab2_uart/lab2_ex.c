/**
 * Copyright (c) 2023 Fuzzi Pi.
 */

#include <stdio.h>
#include "pico/stdlib.h"

#define UART_ID uart1
#define BAUD_RATE 115200

#define UART_TX_PIN 8
#define UART_RX_PIN 9
#define BTN_PIN 22

int main() {
    stdio_init_all();

    // Init UART 
    uart_init(UART_ID, BAUD_RATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
    // Init button vars
    gpio_init(BTN_PIN);
    gpio_set_dir(BTN_PIN, GPIO_IN);
    gpio_pull_up(BTN_PIN);

    char toSend = 'A';
    while (true)
    {
        // Recieving Character
        // If recieved character is ascii code 49 '1'
        if (uart_is_readable(UART_ID))
        {
            char toReceive = uart_getc(UART_ID);
            // 49 is ascii value of '1'
            if (toReceive == 49)
                printf("2\n");
            else
            {
                // lowercase ascii characters are 32 after uppercase characters
                // not checking if the recieved character is an uppercase letter because the only characters able to be sent are '1' and uppercase letters
                toReceive += 32;
                printf("%c\n", toReceive);
            }
        }

        // Sending Character
        if (gpio_get(BTN_PIN))
            uart_putc_raw(UART_ID, '1');
        else 
        {
            uart_putc_raw(UART_ID, toSend);
            // using ascii values to update values. 90 is 'Z', 65 is 'A'
            ++toSend;
            toSend = toSend > 90 ? 65 : toSend;
        }
        
        sleep_ms(1000);
    }
    return 0;
}
