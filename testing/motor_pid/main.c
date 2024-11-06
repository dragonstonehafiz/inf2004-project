#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include <stdio.h>
#include "wheels.h"
#include "encoder.h"

#define BTN_DECREASE_SPEED 22
#define BTN_START_TEST 21
#define BTN_INCREASE_SPEED 20
#define BUTTON_DELAY 1500

void init_gpio();
void init_inerrupts();
void irq_handler(uint gpio, uint32_t events);

struct repeating_timer encoderPrintTimer;
/// @brief just prints data i want to see
bool printCallback(struct repeating_timer *t);
/// @brief only register button press after set time. This is so force applied doesn't affect wheels
int64_t buttonDelayCallback(alarm_id_t id, void* user_data);

int main() 
{
    init_gpio();
    init_inerrupts();
    add_repeating_timer_ms(250, printCallback, NULL, &encoderPrintTimer);

    while (true)
        tight_loop_contents();
}

void init_gpio() 
{
    stdio_init_all();

    gpio_init(BTN_DECREASE_SPEED);
    gpio_init(BTN_INCREASE_SPEED);
    gpio_init(BTN_START_TEST);

    init_wheels();
    setupEncoderPins();
    
    set_car_state(CAR_FORWARD);
    set_wheels_duty_cycle(0.f);
}
void init_inerrupts()
{
    gpio_set_irq_enabled_with_callback(BTN_DECREASE_SPEED, GPIO_IRQ_EDGE_FALL, true, &irq_handler);
    gpio_set_irq_enabled_with_callback(BTN_START_TEST, GPIO_IRQ_EDGE_FALL, true, &irq_handler);
    gpio_set_irq_enabled_with_callback(BTN_INCREASE_SPEED, GPIO_IRQ_EDGE_FALL, true, &irq_handler);
    gpio_set_irq_enabled_with_callback(WHEEL_ENCODER_RIGHT_PIN, GPIO_IRQ_EDGE_FALL, true, &irq_handler);
    gpio_set_irq_enabled_with_callback(WHEEL_ENCODER_LEFT_PIN, GPIO_IRQ_EDGE_FALL, true, &irq_handler);
}

void irq_handler(uint gpio, uint32_t events)
{
    if (gpio == BTN_START_TEST)
    {
        // If the wheel is already turning, set target speed to zero
        if (pid_left.duty_cycle != 0.00f)
        {
            set_car_state(CAR_STATIONARY);
            reset_pid();
            pid_left.enabled = false;
            pid_right.enabled = false;
        }
        // If the wheel is stationary, set the target speed to max
        else
            add_alarm_in_ms(BUTTON_DELAY, buttonDelayCallback, NULL, false);
    }
    else if (gpio == WHEEL_ENCODER_LEFT_PIN || gpio == WHEEL_ENCODER_RIGHT_PIN)
    {
        encoderCallback(gpio, events);
        if (gpio == WHEEL_ENCODER_LEFT_PIN)
            pid_right.target_speed = pid_left.current_speed;
    }
}

bool printCallback(struct repeating_timer *t)
{
    printf("\e[1;1H\e[2J");

    // Print
    printf(" left: %02.2f\nright: %02.2f\n", pid_left.current_speed, pid_right.current_speed);
    printf(" left duty: %.2f\nright duty: %0.2f\n", pid_left.duty_cycle, pid_right.duty_cycle);
    return true;
}

int64_t buttonDelayCallback(alarm_id_t id, void* user_data)
{
    resetEncoder();
    set_car_state(CAR_FORWARD);
    // pid_left.target_speed = 18.f;
    // pid_left.enabled = true;
    pid_right.enabled = true;
    set_wheels_duty_cycle(0.5);
    return 0;
}