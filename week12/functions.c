#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include "functions.h"

#define BUTTON_PIN 20
#define LED_PIN 16
#define LED_PIN2 17

static struct repeating_timer timer;

void main_fn() {
    // Example of using a function with two inputs
    int result = example_function(5, 10);
    printf("Example function result: %d\n", result);
}

// Timer callback function
bool repeating_timer_callback(struct repeating_timer *t) {
    static bool led_state = false;
    gpio_put(LED_PIN2, led_state);  // Toggle LED
    led_state = !led_state;  // Flip state for next toggle

    printf("Timer triggered! LED is now %s\n", led_state ? "ON" : "OFF");
    return true;  // Return true to keep the timer repeating
}

// Initialize GPIO (LED and button with interrupt)
void init_gpio() {
    // Initialize LED
    gpio_init(LED_PIN2);
    gpio_set_dir(LED_PIN2, GPIO_OUT);

    // Initialize Button with interrupt
    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_PIN);
    gpio_set_irq_enabled_with_callback(BUTTON_PIN, GPIO_IRQ_EDGE_FALL, true, button_callback);
}

// Button interrupt callback
void button_callback(uint gpio, uint32_t events) {
    printf("Button on GP%d pressed!\n", gpio);
}

// Function to set up PWM on LED with a specific duty cycle
void init_pwm() {
    gpio_set_function(LED_PIN, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(LED_PIN);

    pwm_set_wrap(slice_num, 255);  // 8-bit resolution (0-255)
    pwm_set_chan_level(slice_num, pwm_gpio_to_channel(LED_PIN), (50 * 255) / 100); // 50% duty cycle
    pwm_set_enabled(slice_num, true);

    printf("PWM initialized on GPIO %d with 50%% duty cycle\n", LED_PIN);
}

// Set up and start a repeating timer that triggers every second (1000 ms)
void start_timer() {
    add_repeating_timer_ms(5000, repeating_timer_callback, NULL, &timer);
}

// Example function with two integer inputs and an integer return value
int example_function(int a, int b) {
    int result = a + b;
    printf("example_function: %d + %d = %d\n", a, b, result);
    return result;
}
