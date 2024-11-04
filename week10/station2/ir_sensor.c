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

// Thresholds and Duty Cycles
#define BLACK_THRESHOLD 500  // Threshold for detecting black
#define WHITE_THRESHOLD 1000 // Threshold for detecting white
#define DUTY_CYCLE_MAX 0.4f  // Max duty cycle

// Correction Factors
static float duty_cycle_left = DUTY_CYCLE_MAX;
static float duty_cycle_right = DUTY_CYCLE_MAX;
static float left_factor = 1.0f;
static float right_factor = 1.0f;

#define LEFT_SIDE 0
#define RIGHT_SIDE 1
static int prev_side = RIGHT_SIDE;

void reset_ir_sensor_status()
{
    idx = 0;
    pulse_start = 0;
    memset(bar_space_widths, 0, sizeof(bar_space_widths));
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
    int curr_state;
    int state_changed;
    uint32_t state_change_time;
} Transaction;

Transaction barcode_transaction;
Transaction line_tracing_transaction;

void create_barcode_transaction(uint gpio, uint32_t events)
{
    barcode_transaction.curr_state = (events & GPIO_IRQ_EDGE_RISE) ? WHITE_DETECTED : BLACK_DETECTED;
    barcode_transaction.state_changed = true;
    barcode_transaction.state_change_time = time_us_32();
}

void create_line_tracing_transaction(uint gpio, uint32_t events)
{
    line_tracing_transaction.curr_state = (events & GPIO_IRQ_EDGE_RISE) ? BLACK_DETECTED : WHITE_DETECTED;
    line_tracing_transaction.state_changed = true;
    line_tracing_transaction.state_change_time = time_us_32();
}

typedef void (*MovementCallback)(uint8_t movement_state, float duty_cycle_left, float duty_cycle_right);

void handle_line_tracing(MovementCallback callback)
{
    if (!line_tracing_transaction.state_changed)
        return;
    line_tracing_transaction.state_changed = false;

    if (line_tracing_transaction.curr_state == BLACK_DETECTED)
    {
        // On black line: move straight and reset correction factors
        duty_cycle_left = DUTY_CYCLE_MAX;
        duty_cycle_right = DUTY_CYCLE_MAX;
        prev_side = prev_side == RIGHT_SIDE ? LEFT_SIDE : RIGHT_SIDE;
        callback(CAR_FORWARD, duty_cycle_left, duty_cycle_right);
    }
    else
    {
        // On white surface: oscillate to regain alignment
        if (prev_side == LEFT_SIDE)
        {
            duty_cycle_left -= (0.3f * left_factor);
            left_factor *= 0.5f; // Halve the left correction factor
            printf("LS -> Left: %f, Right: %f\n", duty_cycle_left, duty_cycle_right);
            callback(CAR_TURN_LEFT_AND_FORWARD, duty_cycle_left, duty_cycle_right);
        }
        else
        {
            duty_cycle_right -= (0.3f * right_factor);
            right_factor *= 0.5f; // Halve the right correction factor
            printf("RS -> Left: %f, Right: %f\n", duty_cycle_left, duty_cycle_right);
            callback(CAR_TURN_RIGHT_AND_FORWARD, duty_cycle_left, duty_cycle_right);
        }
    }
}

void ir_sensor_init()
{
    adc_init();
    adc_gpio_init(IR_SENSOR_PIN);
    adc_select_input(1);

    gpio_init(PULSE_PIN_BARCODE);
    gpio_set_dir(PULSE_PIN_BARCODE, GPIO_IN);

    reset_ir_sensor_status();
}

// Barcode handling remains the same
void handle_barcode()
{
    if (!barcode_transaction.state_changed)
        return;
    barcode_transaction.state_changed = false;

    uint32_t current_time = barcode_transaction.state_change_time;
    uint32_t pulse_width = current_time - pulse_start;

    cancel_repeating_timer(&timer);
    add_repeating_timer_us(TIMEOUT_THRESHOLD_US, inactivity_timeout_callback, NULL, &timer);

    if (pulse_width > PULSE_WIDTH_THRESHOLD)
    {
        if ((barcode_transaction.curr_state == BLACK_DETECTED && idx > 0) ||
            (barcode_transaction.curr_state == WHITE_DETECTED && pulse_start > 0))
            bar_space_widths[idx++] = pulse_width;
        pulse_start = current_time;
    }
}
