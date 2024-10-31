#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include <stdio.h>
#include "wheels.h"

#define BTN_21_PIN 21

// Variables for tracking what step of the test we are at
enum STATION_1_STATE
{
    STATION_1_STATIONARY = 0,
    STATION_1_FIRST_PART, // Moving until object found 10cm away
    STATION_1_TURN, // Turn 90 degrees to the right
    STATION_1_90_CM // Move forward 90cm in 5 seconds
};
uint8_t station1_state = STATION_1_STATIONARY;
/// @brief this function can be called for changing state (so I don't have to rewrite the code in different parts) 
void change_state(uint8_t next_state);

// Timer variables and functions to manage polling of devices
struct repeating_timer pid_timer;
struct repeating_timer ultrasonic_timer;
/// @brief checks the distance to the object in front of the car. If less than 10, stop 
bool ultrasonic_sensor_callback(struct repeating_timer *t);
int64_t wheel_encoder_callback(alarm_id_t id, void* user_data);

void change_state(uint8_t next_state)
{
    station1_state = next_state;
    switch (next_state)
    {
        case STATION_1_FIRST_PART:
            // Move the car forward at max speed
            set_car_state(CAR_FORWARD);
            set_wheels_duty_cycle(1.f);
            // Start timer
            add_repeating_timer_ms(250, ultrasonic_sensor_callback, NULL, &ultrasonic_timer);
            break;
        case STATION_1_TURN:
            station1_state = STATION_1_TURN;
            cancel_repeating_timer(&ultrasonic_timer);
            set_car_state(CAR_STATIONARY);
            break;
        case STATION_1_90_CM:
            // Move the car forward at max speed
            set_car_state(CAR_FORWARD);
            set_wheels_duty_cycle(1.f);
    }
}

void init_gpio();
void irq_handler(uint gpio, uint32_t events);

int main() 
{
    init_gpio();

    while (true) 
        tight_loop_contents();
}

void init_gpio() 
{
    stdio_init_all();
    init_wheels();

    gpio_set_irq_enabled_with_callback(BTN_21_PIN, GPIO_IRQ_EDGE_FALL, true, &irq_handler);
}
void irq_handler(uint gpio, uint32_t events)
{
    if (gpio == BTN_21_PIN)
    {
        if (station1_state == STATION_1_STATIONARY)
            change_state(STATION_1_FIRST_PART);
    }
}

bool ultrasonic_sensor_callback(struct repeating_timer *t)
{
    // Checks the current state of the test. If we are not at the first part of the test (Moving until object found 10cm away),
    // stop this timer (because we have no reason to check distance otherwise)
    if (station1_state == STATION_1_FIRST_PART)
    {
        float distance_to_item = 25.f; // = INSERT CODE HERE 
        if (distance_to_item <= 10.f)
            change_state(STATION_1_TURN);
    }
    else 
    {
        // Realistically, we should never come here, but just to be safe...
        cancel_repeating_timer(&ultrasonic_timer);
    }
    return true;
}
int64_t wheel_encoder_callback(alarm_id_t id, void* user_data)
{
    
}