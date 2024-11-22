#include "setup.h"
#include "states.h"
#include "pins.h"
#include "encoder.h"
#include "ultrasonic.h"
#include "wheels.h"

#define BUTTON_21 21

extern uint8_t currState;

void init_gpio()
{
    stdio_init_all();
    init_wheels();
    setupEncoderPins();
    setupUltrasonicPins();
}
void init_interrupts()
{
    gpio_set_irq_enabled_with_callback(BUTTON_21, GPIO_IRQ_EDGE_FALL, true, &irq_handler);
    gpio_set_irq_enabled_with_callback(WHEEL_ENCODER_RIGHT_PIN, GPIO_IRQ_EDGE_FALL, true, &irq_handler);
    gpio_set_irq_enabled_with_callback(WHEEL_ENCODER_LEFT_PIN, GPIO_IRQ_EDGE_FALL, true, &irq_handler);
}
void irq_handler(uint gpio, uint32_t events)
{
    if (gpio == BUTTON_21)
    {
        if (currState == STATE_INITIAL)
            changeState(STATE_CONNECTING);
    }
    else if (gpio == WHEEL_ENCODER_RIGHT_PIN)
    {
        encoderCallback(gpio, events);
    }
    else if (gpio == WHEEL_ENCODER_LEFT_PIN)
    {
        encoderCallback(gpio, events);
        if (pid_right.enabled)
            pid_right.target_speed = pid_left.current_speed;
    }

}
