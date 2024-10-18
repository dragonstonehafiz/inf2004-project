/**
 * I would like to thank my lord and saviour, ChatGPT for trying its best (even though its code didn't work).
 */

#include <stdio.h>
#include "pico/stdlib.h"

#define BTN_PIN 21

volatile uint32_t elapsedTime = 0;
volatile bool isTimerActive = false;
struct repeating_timer timer;

int64_t debounce_callback(alarm_id_t id, void* user_data);
bool elapsed_timer_callback(alarm_id_t id, void *user_data);
bool repeating_timer_callback(struct repeating_timer *t);

// "LEVEL_LOW",  0x1
// "LEVEL_HIGH", 0x2
// "EDGE_FALL",  0x4
// "EDGE_RISE"   0x8

void gpio_irq_handler(uint gpio, uint32_t events) { 
    if (gpio != BTN_PIN) 
        return;

    // If the button is pressed, and the timer hasn't already been started
    if (events & 0x04 && !isTimerActive) {
    // Timer is called every second
        printf("%d\n", elapsedTime);
        isTimerActive = true;
        add_repeating_timer_ms(-1000, repeating_timer_callback, NULL, &timer);
    }
    else {
        cancel_repeating_timer(&timer);
        elapsedTime = 0;
        isTimerActive = false;
        printf("%d\n", elapsedTime);
    }
    
    // Disable the interrupt
    gpio_set_irq_enabled(BTN_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, false);

    // In 0.5s, Re-enable the button interrupt
    add_alarm_in_ms(
        250,
        debounce_callback,
        NULL,
        false
    );
}

int64_t debounce_callback(alarm_id_t id, void *user_data) {
    // Reset the interrupt.
    gpio_set_irq_enabled(BTN_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true);
    return 0;
}
bool repeating_timer_callback(struct repeating_timer *t) {
    elapsedTime += 1;
    printf("%d\n", elapsedTime);
    return true;
}
int main() {
    // 1. There is an interrupt tied with the button state. When the button goes up/down, the interrupt function is called.
    //
    // 2.
    //    a. If the button is PRESSED (edge fall), start a 0.25s timer that adds to elapsed time
    //    b. If the button is Released (edge rise), stop the timer.
    //
    // 3. In the interrupt function, The button interrupt is disabled. After that, a timer interrupt is started (lasts 500ms(when I wrote this part)).
    //    3.1 When this timer ends, the debounce_callback() function is called. This function re enables the btn interrupt.


    stdio_init_all();

    gpio_init(BTN_PIN);
    gpio_set_dir(BTN_PIN, GPIO_IN);
    gpio_pull_up(BTN_PIN);

    gpio_set_irq_enabled_with_callback(
        BTN_PIN,
        GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE,
        true,
        &gpio_irq_handler
    );

    while (true)
        tight_loop_contents();

    return 0;
}
