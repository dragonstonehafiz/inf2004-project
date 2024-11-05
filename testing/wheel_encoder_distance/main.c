#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include <stdio.h>
#include "wheels.h"
#include "encoder.h"

#define BTN_DECREASE_SPEED 22
#define BTN_START_TEST 21
#define BTN_INCREASE_SPEED 20
#define TEST_TIME 1500

float duty_cycle = 0.5f;
bool test_active = false;

void init_gpio();
void init_inerrupts();
void irq_handler(uint gpio, uint32_t events);

/// @brief this callback is called after 1.5s after the test starts
int64_t end_test_callback(alarm_id_t id, void* user_data);
struct repeating_timer encoderPrintTimer;
/// @brief checks the distance to the object in front of the car. If less than 10, stop 
bool encoderPrintCallback(struct repeating_timer *t);


int main() 
{
    init_gpio();
    init_inerrupts();
    add_repeating_timer_ms(250, encoderPrintCallback, NULL, &encoderPrintTimer);

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
    set_wheels_duty_cycle(duty_cycle);
    setupEncoderPins();
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
    if (!test_active)
    {
        if (gpio == BTN_DECREASE_SPEED)
        {
            duty_cycle -= 0.25;
            if (duty_cycle < 0.f)
                duty_cycle = 0.0f;
            set_wheels_duty_cycle(duty_cycle);
        }
        else if (gpio == BTN_START_TEST)
        {
            set_car_state(CAR_FORWARD);
            test_active = true;
            add_alarm_in_ms(TEST_TIME, end_test_callback, NULL, false);
            leftNotchCount = 0;
            rightNotchCount = 0;
        }
        else if (gpio == BTN_INCREASE_SPEED)
        {
            duty_cycle += 0.25;
            if (duty_cycle > 1.f)
                duty_cycle = 1.f;
            set_wheels_duty_cycle(duty_cycle);
        }
    }
    if (gpio == WHEEL_ENCODER_LEFT_PIN || gpio == WHEEL_ENCODER_RIGHT_PIN)
    {
        encoderCallback(gpio, events);
    }
}

bool encoderPrintCallback(struct repeating_timer *t)
{
    if (test_active)
    {
        printf("leftSpeed:%0.2f, leftDist:%0.2f, leftNotchCount:%d\n", leftEncoderSpeed, leftTotalDistance, leftNotchCount);
        printf("rightSpeed:%0.2f, rightDist:%0.2f, rightNotchCount:%d\n", rightEncoderSpeed, rightTotalDistance, rightNotchCount);
    }
    return true;
}

int64_t end_test_callback(alarm_id_t id, void* user_data)
{
    set_car_state(CAR_STATIONARY);
    test_active = false;
    return 0;
}