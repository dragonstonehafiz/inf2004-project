#ifndef WHEELS_H_
#define WHEELS_H_

#include "motor.h"
#include "pins.h"

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
    bool turning; // this flag is used to make sure we don't try to change duty cycle if wheel is idle
} PID_VAR;
// Variables for controling pid speed
PID_VAR pid_left = {.current_speed = 1.f, .target_speed = 1.f, .duty_cycle = 0.f, .integral = 0.f, .prev_error = 0.f, .turning = false};
PID_VAR pid_right = {.current_speed = 1.f, .target_speed = 1.f, .duty_cycle = 0.f, .integral = 0.f, .prev_error = 0.f, .turning = false};

/// @brief initializes all pins and pwm for both motors
void init_wheels();
/// @brief sets what the car should be doing. (turning left, going forward, stationary, etc...)
/// @param nextState (CAR_STATIONARY, CAR_FORWARD, CAR_BACKWARD, CAR_TURN_RIGHT, CAR_TURN_LEFT)
void set_car_state(uint8_t nextState);
/// @brief sets the duty cycle of both wheels to the same value
/// @param dutyCycle the duty cycle to set the wheels to
void set_wheels_duty_cycle(float dutyCycle);
/// @brief sets the duty cycle for the left wheel
/// @param dutyCycle the duty cycle to set the wheel to
void set_left_wheel_duty_cycle(float dutyCycle);
/// @brief sets the duty cycle for the right wheel
/// @param dutyCycle the duty cycle to set the wheel to
void set_right_wheel_duty_cycle(float dutyCycle);

struct repeating_timer pid_timer;
/// @brief moves the current speed to the targets speed and changes duty_cycle accordingly
void compute_wheel_duty_cycle(PID_VAR * pid);
/// @brief This is the function that will be to use a timer to calculate pid
bool pid_timer_callback(struct repeating_timer *t);
/// @brief Starts the pid timer
void start_pid();
/// @brief Ends the pid timer
void end_pid();

void init_wheels()
{
    setup_motor(WHEEL_LEFT_PWN_PIN, WHEEL_LEFT_OUT_PIN_1, WHEEL_LEFT_OUT_PIN_2);
    setup_motor(WHEEL_RIGHT_PWN_PIN, WHEEL_RIGHT_OUT_PIN_1, WHEEL_RIGHT_OUT_PIN_2);

    set_car_state(CAR_STATIONARY);
    set_motor_pwm_duty_cycle(WHEEL_LEFT_PWN_PIN, 0.f);
    set_motor_pwm_duty_cycle(WHEEL_RIGHT_PWN_PIN, 0.f);
}
void set_car_state(uint8_t nextState)
{
    switch (nextState)
    {
    case CAR_STATIONARY:
        set_motor_stop(WHEEL_LEFT_OUT_PIN_1, WHEEL_LEFT_OUT_PIN_2);
        set_motor_stop(WHEEL_RIGHT_OUT_PIN_1, WHEEL_RIGHT_OUT_PIN_2);
        pid_left.turning = false;
        pid_right.turning = false;
        break;
    case CAR_FORWARD:
        // When moving forward, right wheel is clockwise, left wheel is counter clockwise
        set_motor_direction(WHEEL_LEFT_OUT_PIN_1, WHEEL_LEFT_OUT_PIN_2, false);
        set_motor_direction(WHEEL_RIGHT_OUT_PIN_1, WHEEL_RIGHT_OUT_PIN_2, true);
        pid_left.turning = true;
        pid_right.turning = true;
        break;
    case CAR_BACKWARD:
        // When moving forward, right wheel is counter clockwise, left wheel is clockwise
        set_motor_direction(WHEEL_LEFT_OUT_PIN_1, WHEEL_LEFT_OUT_PIN_2, true);
        set_motor_direction(WHEEL_RIGHT_OUT_PIN_1, WHEEL_RIGHT_OUT_PIN_2, false);
        pid_left.turning = true;
        pid_right.turning = true;
        break;
    case CAR_TURN_RIGHT:
        // When turning right, right wheel is stationary, left wheel is counter clockwise
        set_motor_stop(WHEEL_RIGHT_OUT_PIN_1, WHEEL_RIGHT_OUT_PIN_2);
        set_motor_direction(WHEEL_LEFT_OUT_PIN_1, WHEEL_LEFT_OUT_PIN_2, false);
        pid_left.turning = true;
        pid_right.turning = false;
        break;
    case CAR_TURN_LEFT:
        // When turning right, left wheel is stationary, right wheel is clockwise
        set_motor_stop(WHEEL_LEFT_OUT_PIN_1, WHEEL_LEFT_OUT_PIN_2);
        set_motor_direction(WHEEL_RIGHT_OUT_PIN_1, WHEEL_RIGHT_OUT_PIN_2, true);
        pid_left.turning = false;
        pid_right.turning = true;
        break;
    case CAR_TURN_LEFT_FORWARD:
        set_motor_direction(WHEEL_LEFT_OUT_PIN_1, WHEEL_LEFT_OUT_PIN_2, false);
        set_motor_direction(WHEEL_RIGHT_OUT_PIN_1, WHEEL_RIGHT_OUT_PIN_2, true);
        pid_left.turning = true;
        pid_right.turning = true;
        set_motor_pwm_duty_cycle(WHEEL_LEFT_PWN_PIN, 0.3f);
        set_motor_pwm_duty_cycle(WHEEL_RIGHT_PWN_PIN, 0.5f);
        break;
    case CAR_TURN_RIGHT_FORWARD:
        set_motor_direction(WHEEL_LEFT_OUT_PIN_1, WHEEL_LEFT_OUT_PIN_2, false);
        set_motor_direction(WHEEL_RIGHT_OUT_PIN_1, WHEEL_RIGHT_OUT_PIN_2, true);
        pid_left.turning = true;
        pid_right.turning = true;
        set_motor_pwm_duty_cycle(WHEEL_LEFT_PWN_PIN, 0.5f);
        set_motor_pwm_duty_cycle(WHEEL_RIGHT_PWN_PIN, 0.3f);
        break;
    default:
        break;
    }
}
void set_wheels_duty_cycle(float dutyCycle)
{
    set_motor_pwm_duty_cycle(WHEEL_LEFT_PWN_PIN, dutyCycle);
    set_motor_pwm_duty_cycle(WHEEL_RIGHT_PWN_PIN, dutyCycle);
}
void set_left_wheel_duty_cycle(float dutyCycle)
{
    set_motor_pwm_duty_cycle(WHEEL_LEFT_PWN_PIN, dutyCycle);
}
void set_right_wheel_duty_cycle(float dutyCycle)
{
    set_motor_pwm_duty_cycle(WHEEL_RIGHT_PWN_PIN, dutyCycle);
}

void compute_wheel_duty_cycle(PID_VAR * pid)
{
    float error = pid->target_speed - pid->current_speed;
    pid->integral += error;
    float derivative = error - pid->prev_error;

    // float Kp = 0.1, Ki = 0.01, Kd = 0.005;
    pid->duty_cycle += 0.1 * error + 0.01 * (pid->integral) + 0.005 * derivative;

    // Clamp the duty cycle to the range [0, 1]
    if (pid->duty_cycle > 1.0)
        pid->duty_cycle = 1.0;
    else if (pid->duty_cycle < 0)
        pid->duty_cycle = 0;

    pid->prev_error = error;
}
bool pid_timer_callback(struct repeating_timer *t)
{
    // calculate the new duty cycle of the left wheel
    compute_wheel_duty_cycle(&pid_left);
    set_left_wheel_duty_cycle(pid_left.duty_cycle);
    //
    compute_wheel_duty_cycle(&pid_right);
    set_right_wheel_duty_cycle(pid_right.duty_cycle);
    return true;
}
void start_pid()
{
    add_repeating_timer_ms(100, pid_timer_callback, NULL, &pid_timer);
}
void end_pid()
{
    cancel_repeating_timer(&pid_timer);
}

#endif