#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include <stdio.h>
#include "wheels.h"
#include "encoder.h"

#define BTN_DECREASE_SPEED 22
#define BTN_START_TEST 21
#define BTN_INCREASE_SPEED 20
#define TEST_TIME 5000

void init_gpio();
void init_inerrupts();
void irq_handler(uint gpio, uint32_t events);

struct repeating_timer encoderPrintTimer;
/// @brief checks the distance to the object in front of the car. If less than 10, stop 
bool encoderPrintCallback(struct repeating_timer *t);


int main() 
{
    init_gpio();
    init_inerrupts();
    add_repeating_timer_ms(250, encoderPrintCallback, NULL, &encoderPrintTimer);

    while (true) 
    {
        checkIfStopped();
        tight_loop_contents();
    }
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
            reset_pid();
            set_car_state(CAR_STATIONARY);
            pid_right.enabled = false;
        }
        // If the wheel is stationary, set the target speed to max
        else
        {
            set_car_state(CAR_FORWARD);
            set_wheels_duty_cycle(0.9f);
            pid_right.enabled = true;
            resetEncoder();
        }
    }
    if (gpio == WHEEL_ENCODER_LEFT_PIN || gpio == WHEEL_ENCODER_RIGHT_PIN)
    {
        encoderCallback(gpio, events);
        pid_right.target_speed = pid_left.current_speed;
    }
}

bool encoderPrintCallback(struct repeating_timer *t)
{
    // printf("leftRPM:%0.2f, leftDist:%0.2f, leftNotchCount:%d\n", leftEncoderSpeed, leftTotalDistance, leftNotchCount);
    // printf("rightRPM:%0.2f, rightDist:%0.2f, rightNotchCount:%d\n", rightEncoderSpeed, rightTotalDistance, rightNotchCount);
    return true;
}