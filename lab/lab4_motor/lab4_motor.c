#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include <stdio.h>

// Define GPIO pins
#define PWM_PIN 2     // GP2 for PWM
#define DIR_PIN1 0    // GP0 for direction
#define DIR_PIN2 1    // GP1 for direction

uint16_t motor_channel_level = 1000;
struct repeating_timer timer;

bool increase_speed_callback(struct repeating_timer *t)
{
    motor_channel_level += 1000;
    if (motor_channel_level > 12500)
        motor_channel_level = 1000;
    // Get the PWM slice for the specified GPIO (GPIO 2 in this case)
    uint slice_num = pwm_gpio_to_slice_num(PWM_PIN);
    pwm_set_chan_level(slice_num, PWM_CHAN_A, motor_channel_level);
    printf("%u", motor_channel_level);
    return true;
}

int main() {
    // Initialize GPIO pins for direction control
    gpio_init(DIR_PIN1);
    gpio_init(DIR_PIN2);
    gpio_set_dir(DIR_PIN1, GPIO_OUT);
    gpio_set_dir(DIR_PIN2, GPIO_OUT);

    // Set PWM_PIN (GPIO 2) to PWM function
    gpio_set_function(PWM_PIN, GPIO_FUNC_PWM);
    
    // Get the PWM slice for the specified GPIO (GPIO 2 in this case)
    uint slice_num = pwm_gpio_to_slice_num(PWM_PIN);
    
    // Set PWM frequency (adjust as needed)
    pwm_set_clkdiv(slice_num, 100.0f);  // Divider to control the PWM frequency
    pwm_set_wrap(slice_num, 12500);     // Set the wrap value to control frequency
    
    // Set the duty cycle (50% in this case)
    pwm_set_chan_level(slice_num, PWM_CHAN_A, motor_channel_level); 
    
    // Enable the PWM output
    pwm_set_enabled(slice_num, true);

    // Increase speed of motor every 1 second
    add_repeating_timer_ms(-1000, increase_speed_callback, NULL, &timer);
    
    // Turn One Direction
    gpio_put(DIR_PIN1, 1);
    gpio_put(DIR_PIN2, 0);

    // Turn Other Direction
    //gpio_put(DIR_PIN1, 0);
    //gpio_put(DIR_PIN2, 1);

    // Control motor direction
    while (true) {
        tight_loop_contents();
    }

    return 0;
}
