#ifndef ULTRASONIC_H
#define ULTRASONIC_H

#include "pico/stdlib.h"
#include <stdint.h> 
#include <stdio.h>
#include <stdlib.h>
#include "hardware/gpio.h"
#include "hardware/timer.h"
#include "hardware/irq.h"
#include <math.h>
#include "pins.h"

// Interval in milliseconds for checking the distance
#define CHECK_INTERVAL_MS 200

#define ECHO_TIMEOUT_US 10000

// Define distance bounds for valid measurements
#define MIN_DISTANCE_CM 2.0
#define MAX_DISTANCE_CM 100.0

// Structure for the Kalman filter's state variables
typedef struct {
    double q;   // Process noise covariance
    double r;   // Measurement noise covariance
    double p;   // Error covariance
    double x;   // Estimated value
    double k;   // Kalman gain
} kalman_state;

// Variables for tracking echo timing and state
static volatile uint64_t echo_start = 0;
static volatile uint64_t echo_duration = 0;
static volatile bool echo_received = false;
static volatile bool echo_timeout = false;

// Kalman filter instance and timeout alarm ID
static alarm_id_t timeout_alarm_id;
static kalman_state *kf = NULL;

// Enhanced Kalman filter update with boundary checking
void kalman_update(kalman_state *state, double measurement) {
    if (state == NULL || measurement < MIN_DISTANCE_CM || measurement > MAX_DISTANCE_CM) {
        return;
    }

    // Calculate innovation (measurement - prediction)
    double innovation = measurement - state->x;

    // Adaptive process noise based on innovation
    if (fabs(innovation) > 10.0) {
        // Temporarily increase process noise for large changes
        state->q *= 3.0;
    } else {
        // Return to normal process noise
        state->q = 1.0;
    }

    // Special handling for measurements around 9.5 cm to increase trust
    if (measurement >= 9.4 && measurement <= 9.8) {
        state->r *= 0.4;  
    }

    // Prediction update with adjusted process noise
    state->p = state->p + state->q;

    // Calculate Kalman gain
    state->k = state->p / (state->p + state->r);

    // More aggressive update for small innovations
    if (fabs(innovation) < 50.0) {
        state->x += state->k * innovation;
    } else {
        // For very large changes, trust the measurement more
        state->x = 0.7 * measurement + 0.3 * state->x;
    }

    // Ensure estimate stays within bounds
    if (state->x < MIN_DISTANCE_CM) state->x = MIN_DISTANCE_CM;
    if (state->x > MAX_DISTANCE_CM) state->x = MAX_DISTANCE_CM;

    // Update error covariance
    state->p = (1 - state->k) * state->p;
}

// Function to initialize the Kalman filter state
kalman_state *kalman_init(double q, double r, double p, double initial_value) {
    kalman_state *state = calloc(1, sizeof(kalman_state));
    if (state == NULL) {
        return NULL;
    }
    
    state->q = q > 0 ? q : 1.0;      // Process noise - lower value for more stable output
    state->r = r > 0 ? r : 0.5;      // Measurement noise - higher value for noisier measurements
    state->p = p > 0 ? p : 1.0;      // Initial estimation error covariance
    state->x = initial_value;        // Initial state estimate
    return state;
}

/*
Parameter tuning:
- Increase `r` if output is too jumpy
- Increase `q` if response is too slow
- Decrease `q` if output is too noisy
*/

static void echo_pin_handler(uint gpio, uint32_t events);
static int64_t timeout_callback(alarm_id_t id, void *user_data);

// Callback for echo timeout
static int64_t timeout_callback(alarm_id_t id, void *user_data) {
    echo_timeout = true;
    echo_duration = 0; // Reset echo duration on timeout
    return 0;
}

// Handler for echo pin interrupt to track rising and falling edges
static void echo_pin_handler(uint gpio, uint32_t events) {
    if (events & GPIO_IRQ_EDGE_RISE) {
        echo_start = time_us_64();
        echo_received = false;
        echo_timeout = false;
        timeout_alarm_id = add_alarm_in_us(ECHO_TIMEOUT_US, timeout_callback, NULL, false);
    } else if (events & GPIO_IRQ_EDGE_FALL) {
        if (timeout_alarm_id > 0) cancel_alarm(timeout_alarm_id);
        echo_duration = time_us_64() - echo_start;
        echo_received = true;
    }
}

void setupUltrasonicPins() {
    gpio_init(TRIG_PIN);
    gpio_init(ECHO_PIN);
    gpio_set_dir(TRIG_PIN, GPIO_OUT);
    gpio_set_dir(ECHO_PIN, GPIO_IN);
    gpio_set_irq_enabled_with_callback(ECHO_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &echo_pin_handler);
}

void setupBuzzerPin() {
    gpio_init(BUZZER_PIN);
    gpio_set_dir(BUZZER_PIN, GPIO_OUT);
}

// Initialize the Kalman filter for the ultrasonic sensor
void initializeKalmanFilter() {
    kf = kalman_init(1.0, 0.5, 1.0, 10.0);  // Adjust initial parameters as needed
    if (kf == NULL) {
        printf("Failed to initialize Kalman filter\n");
    }
}

void triggerPulse() {
    gpio_put(TRIG_PIN, 0);
    sleep_us(2);
    gpio_put(TRIG_PIN, 1);
    sleep_us(10);
    gpio_put(TRIG_PIN, 0);
    echo_received = false;
    echo_timeout = false;
    echo_duration = 0;
}

float getCm() {
    if (echo_timeout) {
        return -1.0;
    }
    if (!echo_received) {
        return -1.0;
    }

    float raw_distance = (float)echo_duration / 57.5;

    if (raw_distance < MIN_DISTANCE_CM || raw_distance > MAX_DISTANCE_CM) {
        return -1.0;
    }

    kalman_update(kf, raw_distance);

    float filtered_distance = (float)kf->x;

    if (filtered_distance > 9.3){
        filtered_distance -= 0.25;
    } else {
        filtered_distance -= 0.2;
    }

    return filtered_distance;
}

#endif 