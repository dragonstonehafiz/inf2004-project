/**
 * I would like to thank my lord and saviour, ChatGPT for trying its best (even though its code didn't work).
 */

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"

#define PIN_ADC 26
#define PIN_PWM 0

struct repeating_timer timer;

// Function to set up the PWM
void setup_pwm(uint gpio, float freq, float duty_cycle) {
    // Set the GPIO function to PWM
    gpio_set_function(gpio, GPIO_FUNC_PWM);

    // Find out which PWM slice is connected to the specified GPIO
    uint slice_num = pwm_gpio_to_slice_num(gpio);

    // Calculate the PWM frequency and set the PWM wrap value
    float clock_freq = 125000000.0f;  // Default Pico clock frequency in Hz
    uint32_t divider = clock_freq / (freq * 65536);  // Compute divider for given frequency
    pwm_set_clkdiv(slice_num, divider);

    // Set the PWM wrap value (maximum count value)
    pwm_set_wrap(slice_num, 65535);  // 16-bit counter (0 - 65535)

    // Set the duty cycle
    pwm_set_gpio_level(gpio, (uint16_t)(duty_cycle * 65536));

    // Enable the PWM
    pwm_set_enabled(slice_num, true);
}

bool sample_adc_callback(struct repeating_timer *t)
{
    // Get current time in microseconds since boot
    uint64_t time_us = time_us_64();

    // Convert to hours, minutes, seconds, and milliseconds
    uint32_t milliseconds = (time_us / 1000) % 1000;
    uint32_t seconds = (time_us / 1000000) % 60;
    uint32_t minutes = (time_us / (1000000 * 60)) % 60;

    uint16_t result = adc_read();
    printf("%02u:%02u:%03u -> ADC Value: %u\n", minutes, seconds, milliseconds, result);
    return true;
}

int main() {
    stdio_init_all();

    // Start Timer to print adc value
    add_repeating_timer_ms(25, sample_adc_callback, NULL, &timer);
    
    // Init ADC Variables
    adc_init();
    adc_gpio_init(PIN_ADC);
    adc_select_input(0);

    // Init PWM Variables
    setup_pwm(PIN_PWM, 20, 0.5f);
    gpio_set_function(PIN_PWM, GPIO_FUNC_PWM);

    while (true)
        tight_loop_contents();

    return 0;
}
