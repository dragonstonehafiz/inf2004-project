#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include <stdio.h>
#include "motor.h"
#include "wheel_encoder.h"

#define BTN_1_PIN 20
#define BTN_2_PIN 21
#define BTN_3_PIN 22

uint8_t duty_cycle = 0;
uint64_t last_time_r;

void init_gpio();

void irq_handler(uint gpio, uint32_t events);

void irq_btn(uint gpio);

int main() 
{
    init_gpio();

    float target_speed = 25.f;
    float integral = 0;
    float prev_error = 0;

    while (true) 
    {
        tight_loop_contents();
        // compute_pid(&target_speed, &current_speed, &integral, &prev_error);
    }
}

void init_gpio() 
{
    stdio_init_all();

    init_motor();
    init_wheel_encoder();

    gpio_set_irq_enabled_with_callback(WHEEL_ENCODER_RIGHT_PIN, GPIO_IRQ_EDGE_FALL, true, &irq_handler);
    gpio_set_irq_enabled_with_callback(BTN_1_PIN, GPIO_IRQ_EDGE_FALL, true, &irq_handler);
    gpio_set_irq_enabled_with_callback(BTN_2_PIN, GPIO_IRQ_EDGE_FALL, true, &irq_handler);
    gpio_set_irq_enabled_with_callback(BTN_3_PIN, GPIO_IRQ_EDGE_FALL, true, &irq_handler);
}

void irq_handler(uint gpio, uint32_t events)
{
    if (gpio == BTN_1_PIN || gpio == BTN_2_PIN || gpio == BTN_3_PIN)
        irq_btn(gpio);
    else if (gpio == WHEEL_ENCODER_RIGHT_PIN)
        irq_ir_sensor(WHEEL_ENCODER_RIGHT_PIN);
}

void irq_btn(uint gpio)
{
    if (gpio == BTN_1_PIN) {
        left_wheel_dir = !left_wheel_dir;
        gpio_put(WHEEL_LEFT_OUT_PIN_1, left_wheel_dir);
        gpio_put(WHEEL_LEFT_OUT_PIN_2, !left_wheel_dir);
    }
    else if (gpio == BTN_2_PIN) {
        duty_cycle += 25;
        if (duty_cycle > 100)
            duty_cycle = 0;
        // Divide by 101 because when duty cycle is 100, motor won't turn
        float duty_cycle_percent = duty_cycle / 101.f;
        set_pwm_duty_cycle(WHEEL_LEFT_PWN_PIN, duty_cycle_percent);
        set_pwm_duty_cycle(WHEEL_RIGHT_PWN_PIN, duty_cycle_percent);
    }
    else if (gpio == BTN_3_PIN) {
        right_wheel_dir = !right_wheel_dir;
        gpio_put(WHEEL_RIGHT_OUT_PIN_1, right_wheel_dir);
        gpio_put(WHEEL_RIGHT_OUT_PIN_2, !right_wheel_dir);
    }
}