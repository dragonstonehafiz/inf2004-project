#include "ultrasonic.h"

#include <stdint.h> 
#include <stdio.h>
#include "hardware/irq.h"
#include <math.h>
#include "pins.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"

#define ECHO_TIMEOUT_US 30000

// Variables for interrupt handling
static volatile uint64_t echo_start = 0;
static volatile uint64_t echo_duration = 0;
static volatile bool echo_received = false;
static volatile bool echo_timeout = false;

// Alarm ID for timeout
static alarm_id_t timeout_alarm_id;

// Kalman filter variables
static float distance_estimate = 0.0;
static float kalman_gain = 0.6;
static float process_noise = 0.1;
static float measurement_noise = 0.3;

// Variables used for automatic timer
struct repeating_timer ultrasonicTimer;
float ultrasonicDistance;

void setupUltrasonicPins()
{
    gpio_init(TRIG_PIN);
    gpio_init(ECHO_PIN);
    gpio_set_dir(TRIG_PIN, GPIO_OUT);
    gpio_set_dir(ECHO_PIN, GPIO_IN);
}

void setupBuzzerPin()
{
    gpio_init(BUZZER_PIN);
    gpio_set_dir(BUZZER_PIN, GPIO_OUT);
}

static int64_t timeout_callback(alarm_id_t id, void *user_data)
{
    echo_timeout = true;
    return 0;
}

void echo_pin_handler(uint gpio, uint32_t events)
{
    if (events & GPIO_IRQ_EDGE_RISE)
    {
        echo_start = time_us_64();
        echo_received = false;
        echo_timeout = false;

        // Set timeout alarm
        timeout_alarm_id = add_alarm_in_us(ECHO_TIMEOUT_US, timeout_callback, NULL, false);
    }
    else if (events & GPIO_IRQ_EDGE_FALL)
    {
        // Cancel timeout alarm since we received the echo
        if (timeout_alarm_id > 0)
        {
            cancel_alarm(timeout_alarm_id);
        }

        uint64_t end_time = time_us_64();
        echo_duration = end_time - echo_start;
        echo_received = true;
    }
}

void triggerPulse()
{
    // Generate trigger pulse
    gpio_put(TRIG_PIN, 0);
    sleep_us(2);
    gpio_put(TRIG_PIN, 1);
    sleep_us(10);
    gpio_put(TRIG_PIN, 0);

    // Reset measurement state to await new measurement
    resetMeasurementState();
}

float getCm()
{
    if (!echo_received || echo_timeout) {
        return -1.0;  // Return -1 if no valid echo is received
    }

    float distance = (float)echo_duration / 29.0 / 2.0;

    // Apply calibration for close distances
    distance += 0.1; 

    // Apply linear or polynomial calibration
    if (distance > 9.3 && distance < 9.75) {
       distance = distance * 0.98; 
    } else if (distance < 5.7) {
        distance -= 0.15;
        distance = distance * 0.98 + 0.05; 
    } else if (distance > 8.1 && distance < 9.1) {
        distance += 0.3;
    }

    // Define minimum and maximum reliable distances in centimeters
    const float MIN_DISTANCE_CM = 2.0;
    const float MAX_DISTANCE_CM = 150.0;

    // Check if the distance is within a valid range
    if (distance < MIN_DISTANCE_CM || distance > MAX_DISTANCE_CM) {
        return -1.0;  // Return -1 if out of range
    }

    return applyKalmanFilter(distance);
}

float applyKalmanFilter(float measured_distance)
{
    // Kalman filter update step
    float prediction = distance_estimate + process_noise;
    kalman_gain = prediction / (prediction + measurement_noise);
    distance_estimate = prediction + kalman_gain * (measured_distance - prediction);

    return distance_estimate;
}

static void resetMeasurementState()
{
    // Clear state variables before starting a new measurement
    echo_received = false;
    echo_timeout = false;
    echo_duration = 0;
}