#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include <stdio.h>
#include "wheels.h"
#include "ultrasonic.h"
#include "encoder.h"

#define BTN_START 21
#define ANGLE_TO_TURN 90

/// @brief this function can be called for changing state (so I don't have to rewrite the code in different parts) 
void change_state(uint8_t next_state);
// Variables for tracking what step of the test we are at
enum STATION_1_STATE
{
    STATION_1_IDLE = 0,
    STATION_1_FIRST_PART, // Moving until object found 10cm away
    STATION_1_TURN_IDLE,
    STATION_1_TURN, // Turn 90 degrees to the right
    STATION_1_90_CM_IDLE,
    STATION_1_90_CM // Move forward 90cm in 5 seconds
};
uint8_t station1_state = STATION_1_IDLE;

// Timer variables and functions to manage polling of devices
struct repeating_timer pid_timer;
struct repeating_timer ultrasonic_timer;
/// @brief checks the distance to the object in front of the car. If less than 10, stop 
bool ultrasonic_sensor_callback(struct repeating_timer *t);

// For turning task
// This is how much the left wheel needs to turn when 
float distToTravel = 0.f;

void change_state(uint8_t next_state)
{
    station1_state = next_state;
    set_wheels_duty_cycle(0.f);
    switch (next_state)
    {
        case STATION_1_IDLE:
            set_car_state(CAR_STATIONARY);
            set_wheels_duty_cycle(0.f);
            printf("FIRST PART IDLE\n");
            break;
        case STATION_1_FIRST_PART:
            // Move the car forward at max speed
            set_car_state(CAR_FORWARD);
            set_wheels_duty_cycle(1.f);
            // Start timer
            // add_repeating_timer_ms(250, ultrasonic_sensor_callback, NULL, &ultrasonic_timer);
            printf("FIRST PART\n");
            break;
        case STATION_1_TURN_IDLE:
            // Move the car forward at max speed
            set_car_state(CAR_STATIONARY);
            set_wheels_duty_cycle(0.f);
            printf("TURN IDLE\n");
            break;
        case STATION_1_TURN:
            set_car_state(CAR_TURN_RIGHT);
            station1_state = STATION_1_TURN;
            // cancel_repeating_timer(&ultrasonic_timer);
            set_wheels_duty_cycle(1.f);
            distToTravel = ((float)ANGLE_TO_TURN / 360.f) * 76.0265;
            resetEncoder();
            printf("TURN\n");
            break;
        case STATION_1_90_CM_IDLE:
            set_car_state(CAR_STATIONARY);
            set_wheels_duty_cycle(0.f);
            printf("90 CM IDLE\n");
            break;
        case STATION_1_90_CM:
            set_car_state(CAR_FORWARD);
            // Move the car forward at max speed
            set_wheels_duty_cycle(1.f);
            distToTravel = 90;
            resetEncoder();
            printf("90 CM\n");
            break;
    }
}

void init_gpio();
void init_interrupts();
void irq_handler(uint gpio, uint32_t events);

int main() 
{
    init_gpio();
    init_interrupts();
    change_state(STATION_1_IDLE);

    uint64_t last_distance_check_time = time_us_64();
    while (true) 
    {
        uint64_t current_time = time_us_64();
        uint64_t timediff = current_time - last_distance_check_time;

        if (timediff > CHECK_INTERVAL_MS * 1000 && 
            station1_state == STATION_1_FIRST_PART)
        {
            float distance_to_item = getCm();
            printf("%.2f\n", distance_to_item);
            if (distance_to_item <= 10.f && distance_to_item > 0.0f)
                change_state(STATION_1_TURN_IDLE);

            last_distance_check_time = current_time;
        }
        
    }
    return 1;
}

void init_gpio() 
{
    stdio_init_all();
    init_wheels();
    setupUltrasonicPins();
    setupEncoderPins();
}
void init_interrupts()
{
    gpio_set_irq_enabled_with_callback(BTN_START, GPIO_IRQ_EDGE_FALL, true, &irq_handler);
    gpio_set_irq_enabled_with_callback(WHEEL_ENCODER_RIGHT_PIN, GPIO_IRQ_EDGE_FALL, true, &irq_handler);
    gpio_set_irq_enabled_with_callback(WHEEL_ENCODER_LEFT_PIN, GPIO_IRQ_EDGE_FALL, true, &irq_handler);
}
void irq_handler(uint gpio, uint32_t events)
{
    if (gpio == BTN_START)
    {
        if (station1_state == STATION_1_IDLE)
            change_state(STATION_1_FIRST_PART);
        else if (station1_state == STATION_1_TURN_IDLE)
            change_state(STATION_1_TURN);
        else if (station1_state == STATION_1_90_CM_IDLE)
            change_state(STATION_1_90_CM);
    }
    else if (gpio == WHEEL_ENCODER_LEFT_PIN || gpio == WHEEL_ENCODER_RIGHT_PIN)
    {
        encoderCallback(gpio, events);

        if (leftTotalDistance >= distToTravel)
        {
            switch (station1_state)
            {
                case STATION_1_TURN:
                    change_state(STATION_1_90_CM_IDLE);
                    break;
                case STATION_1_90_CM:
                    change_state(STATION_1_IDLE);
                    break;
                default:
                    break;
            }
        }
    }
}

bool ultrasonic_sensor_callback(struct repeating_timer *t)
{
    // Checks the current state of the test. If we are not at the first part of the test (Moving until object found 10cm away),
    // stop this timer (because we have no reason to check distance otherwise)
    if (station1_state == STATION_1_FIRST_PART)
    {
        float distance_to_item = 12;
        if (distance_to_item <= 10.f && distance_to_item > 0.0f)
            change_state(STATION_1_TURN);
    }
    else 
    {
        // Realistically, we should never come here, but just to be safe...
        cancel_repeating_timer(&ultrasonic_timer);
    }
    return true;
}
