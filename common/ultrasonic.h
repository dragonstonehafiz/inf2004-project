#ifndef ULTRASONIC_H
#define ULTRASONIC_H

#include "pico/stdlib.h"

void setupUltrasonicPins();
void setupBuzzerPin();
void echo_pin_handler(uint gpio, uint32_t events);
void triggerPulse();
float getCm();
static int64_t timeout_callback(alarm_id_t id, void *user_data);
static void resetMeasurementState();
float applyKalmanFilter(float measured_distance);

#endif