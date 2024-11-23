#ifndef WHEELS_H_
#define WHEELS_H_

#include "pico/stdlib.h"

enum CAR_STATE
{
    CAR_STATIONARY = 0,
    CAR_FORWARD,
    CAR_BACKWARD,
    CAR_TURN_RIGHT,
    CAR_TURN_LEFT,
    CAR_TURN_LEFT_FORWARD,
    CAR_TURN_RIGHT_FORWARD,
    NUM_CAR_STATES
};

typedef struct PID_VAR
{
    float current_speed;
    float target_speed;
    float duty_cycle;
    float integral;
    float prev_error;
    bool turning; // turning indicates if the wheel should be turning
    bool enabled; // enabled indicates if pid should set duty cycle 
} PID_VAR;

// Variables for controling pid speed
extern PID_VAR pid_left;
extern PID_VAR pid_right;

/// @brief initializes all pins and pwm for both motors
void init_wheels();
/// @brief sets what the car should be doing. (turning left, going forward, stationary, etc...)
/// @param nextState (CAR_STATIONARY, CAR_FORWARD, CAR_BACKWARD, CAR_TURN_RIGHT, CAR_TURN_LEFT)
void set_car_state(uint8_t nextState);
/// @brief sets the duty cycle of both wheels to the same value
void set_wheels_duty_cycle(float dutyCycle);
/// @brief sets the duty cycle for the left wheel
void set_left_wheel_duty_cycle(float dutyCycle);
/// @brief sets the duty cycle for the right wheel
void set_right_wheel_duty_cycle(float dutyCycle);

/// @brief moves the current speed to the targets speed and changes duty_cycle accordingly
void compute_wheel_duty_cycle(PID_VAR * pid);
/// @brief This is the function that will be to use a timer to calculate pid
bool pid_timer_callback(struct repeating_timer *t);
/// @brief Resets the pid variables for both wheels so no errors and integrals carry over
void reset_pid();

#endif