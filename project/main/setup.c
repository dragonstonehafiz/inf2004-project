#include "setup.h"
#include "states.h"
#include "pins.h"
#include "encoder.h"
#include "ultrasonic.h"
#include "wheels.h"
#include "code39_decoder.h"
#include "ir_sensor.h"

#define BUTTON_21 21

extern volatile uint8_t currState;

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
    gpio_set_irq_enabled_with_callback(ECHO_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &irq_handler);
    gpio_set_irq_enabled_with_callback(PULSE_PIN_BARCODE, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &irq_handler);
    gpio_set_irq_enabled_with_callback(PIN_LINE_TRACING, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &irq_handler);
}
void irq_handler(uint gpio, uint32_t events)
{
    if (gpio == BUTTON_21)
    {
        if (currState == STATE_INITIAL)
            changeState(STATE_CONNECTING);
    }
    else if (gpio == WHEEL_ENCODER_RIGHT_PIN)
        encoderCallback(gpio, events);
    else if (gpio == WHEEL_ENCODER_LEFT_PIN)
    {
        encoderCallback(gpio, events);
        if (pid_right.enabled)
            pid_right.target_speed = pid_left.current_speed;
    }
    else if (gpio == ECHO_PIN)
        echo_pin_handler(gpio, events);

    else if (gpio == PULSE_PIN_BARCODE && getState() == STATE_AUTO)
        create_barcode_transaction(gpio, events);

    else if (gpio == PIN_LINE_TRACING && getState() == STATE_AUTO)
        create_line_tracing_transaction(gpio, events);
}
