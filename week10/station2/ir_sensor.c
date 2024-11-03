// ir_sensor.c
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/timer.h"
#include "hardware/gpio.h"
#include "ir_sensor.h"
#include "code39_decoder.h"

enum CAR_STATE
{
    CAR_STATIONARY = 0,
    CAR_FORWARD,
    CAR_BACKWARD,
    CAR_TURN_RIGHT,
    CAR_TURN_LEFT,
    CAR_TURN_LEFT_AND_FORWARD,
    CAR_TURN_RIGHT_AND_FORWARD,
    NUM_CAR_STATES
};

static struct repeating_timer timer;
static uint32_t pulse_start = 0;

static int64_t bar_space_widths[BARCODE_BITS_LENGTH] = {0};

static int idx = 0;

// Function to reset IR sensor status
void reset_ir_sensor_status()
{
    idx = 0;
    pulse_start = 0;
    memset(bar_space_widths, 0, sizeof(bar_space_widths));
}

void record_segment_callback(uint32_t pulse_width, const char *segment_type)
{
    bar_space_widths[idx++] = pulse_width;

    printf("Recorded %s segment: %d\n", segment_type, pulse_width);
}

bool inactivity_timeout_callback()
{
    if (idx > 0)
        decode_with_direction_check(bar_space_widths);
    reset_ir_sensor_status();

    return false;
}

typedef struct Transaction
{
    int state;
    bool state_change;
    uint32_t state_change_time;
} Transaction;

Transaction barcode_transaction;

void create_barcode_transaction(uint gpio, uint32_t events)
{
    barcode_transaction.state = (events & GPIO_IRQ_EDGE_RISE) ? WHITE_DETECTED : BLACK_DETECTED;
    barcode_transaction.state_change = true;
    barcode_transaction.state_change_time = time_us_32();
}

void handle_barcode()
{
    if (!barcode_transaction.state_change)
        return;
    barcode_transaction.state_change = false;
    uint32_t current_time = barcode_transaction.state_change_time;
    uint32_t pulse_width = current_time - pulse_start;

    // Reset inactivity timer on every transition
    cancel_repeating_timer(&timer);
    add_repeating_timer_us(TIMEOUT_THRESHOLD_US, inactivity_timeout_callback, NULL, &timer);

    if (pulse_width > PULSE_WIDTH_THRESHOLD)
    {
        if ((barcode_transaction.state == BLACK_DETECTED && idx > 0) ||
            (barcode_transaction.state == WHITE_DETECTED && pulse_start > 0))
            bar_space_widths[idx++] = pulse_width;
        pulse_start = current_time;
    }
}

typedef void (*MovementCallback)(uint8_t movement_state);

Transaction line_tracing_transaction;

void create_line_tracing_transaction(uint gpio, uint32_t events)
{
    line_tracing_transaction.state = (events & GPIO_IRQ_EDGE_RISE) ? BLACK_DETECTED : WHITE_DETECTED;
    line_tracing_transaction.state_change = true;
    line_tracing_transaction.state_change_time = time_us_32();
    line_tracing_transaction.state_change = true;
}

void handle_line_tracing(MovementCallback callback)
{
    if (!line_tracing_transaction.state_change)
        return;
    line_tracing_transaction.state_change = false;

    if (line_tracing_transaction.state == BLACK_DETECTED)
    {
        return callback(CAR_TURN_RIGHT_AND_FORWARD);
    }
    else if (line_tracing_transaction.state == WHITE_DETECTED)
    {
        return callback(CAR_TURN_LEFT_AND_FORWARD);
    }
}

void ir_sensor_init()
{
    adc_init();
    adc_gpio_init(IR_SENSOR_PIN);
    adc_select_input(0);

    gpio_init(PULSE_PIN_BARCODE);
    gpio_set_dir(PULSE_PIN_BARCODE, GPIO_IN);

    // Reset sensor status at the start
    reset_ir_sensor_status();
}