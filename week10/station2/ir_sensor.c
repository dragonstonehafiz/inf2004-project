// ir_sensor.c
#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/timer.h"
#include "hardware/gpio.h"
#include "ir_sensor.h"
#include "code39_decoder.h"

static struct repeating_timer timer;
static uint32_t pulse_start = 0;

static int64_t bar_space_widths[BARCODE_BITS_LENGTH] = {0};
static uint32_t narrow_width = UINT32_MAX;

static int idx = 0;

// Function to reset IR sensor status
void reset_ir_sensor_status()
{
    idx = 0;
    narrow_width = UINT32_MAX;
    pulse_start = 0;
    memset(bar_space_widths, 0, sizeof(bar_space_widths));
    // printf("IR sensor status reset.\n");
}

void record_segment_callback(uint32_t pulse_width, const char *segment_type)
{
    bar_space_widths[idx++] = pulse_width;

    if (pulse_width < narrow_width)
    {
        narrow_width = pulse_width;
    }

    // printf("Width of %s segment: %d us\n", segment_type, pulse_width);
}

bool inactivity_timeout_callback()
{
    // printf("Inactivity detected: No state change for 1 second.\n");

    if (idx > 0 && decode_with_direction_check(bar_space_widths, narrow_width))
    {
        // printf("Barcode decoded successfully.\n");
    }
    else
    {
        // printf("Failed to decode barcode.\n");
    }

    // Reset all statuses after decoding or timeout
    reset_ir_sensor_status();
    return false;
}

void handle_edge_transition(uint gpio, uint32_t events)
{
    uint32_t current_time = time_us_32();
    uint32_t pulse_width = current_time - pulse_start;

    // Reset inactivity timer on every transition
    cancel_repeating_timer(&timer);
    add_repeating_timer_us(TIMEOUT_THRESHOLD_US, inactivity_timeout_callback, NULL, &timer);

    if (pulse_width > PULSE_WIDTH_THRESHOLD)
    {
        if (events & GPIO_IRQ_EDGE_RISE)
        {
            // black detected, record the previouis colour and pulse width
            if (idx != 0)
                record_segment_callback(pulse_width, "white");
            pulse_start = current_time;
        }
        else if (events & GPIO_IRQ_EDGE_FALL && pulse_start > 0)
        {
            // white detected, record the previouis colour and pulse width
            record_segment_callback(pulse_width, "black");
            pulse_start = current_time;
        }
    }
}

void ir_sensor_init()
{
    gpio_init(PULSE_PIN);
    gpio_set_dir(PULSE_PIN, GPIO_IN);

    // Reset sensor status at the start
    reset_ir_sensor_status();
}

bool is_wide(int64_t width, int64_t narrow_width)
{
    return width > (narrow_width * WIDE_FACTOR);
}