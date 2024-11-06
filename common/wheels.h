#ifndef WHEELS_H_
#define WHEELS_H_

#include "motor.h"
#include "pins.h"
#include <math.h>

#define MAX_STEP 0.1

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
    bool enabled;
    uint64_t last_time;
} PID_VAR;
// Variables for controling pid speed
PID_VAR pid_left = {.current_speed = 0.f, .target_speed = 0.f, .duty_cycle = 0.f, .integral = 0.f, .prev_error = 0.f, .turning = false, .enabled = false, .last_time=0};
PID_VAR pid_right = {.current_speed = 0.f, .target_speed = 0.f, .duty_cycle = 0.f, .integral = 0.f, .prev_error = 0.f, .turning = false, .enabled = false, .last_time=0};

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
/// @brief Resets the pid variables for both wheels
void reset_pid();

void init_wheels()
{
    setup_motor(WHEEL_LEFT_PWN_PIN, WHEEL_LEFT_OUT_PIN_1, WHEEL_LEFT_OUT_PIN_2);
    setup_motor(WHEEL_RIGHT_PWN_PIN, WHEEL_RIGHT_OUT_PIN_1, WHEEL_RIGHT_OUT_PIN_2);

    set_car_state(CAR_STATIONARY);
    set_wheels_duty_cycle(0.f);

    add_repeating_timer_ms(100, pid_timer_callback, NULL, &pid_timer);
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
    pid_left.duty_cycle = dutyCycle;
    set_motor_pwm_duty_cycle(WHEEL_RIGHT_PWN_PIN, dutyCycle);
    pid_right.duty_cycle = dutyCycle;
}
void set_left_wheel_duty_cycle(float dutyCycle)
{
    set_motor_pwm_duty_cycle(WHEEL_LEFT_PWN_PIN, dutyCycle);
    pid_left.duty_cycle = dutyCycle;
}
void set_right_wheel_duty_cycle(float dutyCycle)
{
    set_motor_pwm_duty_cycle(WHEEL_RIGHT_PWN_PIN, dutyCycle);
    pid_right.duty_cycle = dutyCycle;
}

void compute_wheel_duty_cycle(PID_VAR * pid)
{
    float error = pid->target_speed - pid->current_speed;
    // Ideally both wheels should start at the same duty cycle so their speeds should be close
    // This line makes sure there won'e be too big for the first step
    if (error > 10.f)
        return;
    // Clamping integral because I'm worried it'll get crazy large if wheel is stationary for too long.
    pid->integral += error;
    float derivative = error - pid->prev_error;
    // online formulas have delta time
    uint64_t now = time_us_64();
    float dt = (now - pid->last_time) / 1e6;
    // float Kp = 0.1, Ki = 0.01, Kd = 0.005;
    float step = (0.1 * error + 0.007 * (pid->integral) + 0.005 * derivative) * dt;
    // This function is ideally being called 10 times a second, so we don't want to make the step too big
    if (step > MAX_STEP)
        step = MAX_STEP;
    else if (step < -MAX_STEP)
        step = -MAX_STEP;
    // printf("step: %0.2f, dt:%0.2f\n", step, dt);
    pid->duty_cycle += step;

    // Clamp the duty cycle to the range [0, 1]
    if (pid->duty_cycle > 1.f)
        pid->duty_cycle = 1.f;
    else if (pid->duty_cycle < 0.1)
        pid->duty_cycle = 0.1;

    pid->prev_error = error;
    pid->last_time = now;
}
bool pid_timer_callback(struct repeating_timer *t)
{
    if (pid_left.enabled && pid_left.turning)
    {
        // calculate the new duty cycle of the left wheel
        compute_wheel_duty_cycle(&pid_left);
        set_left_wheel_duty_cycle(pid_left.duty_cycle);
    }
    if (pid_right.enabled && pid_left.turning)
    {
        // calculate duty cycle for right wheel
        compute_wheel_duty_cycle(&pid_right);
        set_right_wheel_duty_cycle(pid_right.duty_cycle);
    }
    return true;
}
void reset_pid()
{
    pid_left.current_speed = 0.f;
    pid_left.target_speed = 0.f;
    pid_left.duty_cycle = 0.f;
    pid_left.integral = 0.f;
    pid_left.prev_error = 0.f;
    pid_left.last_time = time_us_64();

    pid_right.current_speed = 0.f;
    pid_right.target_speed = 0.f;
    pid_right.duty_cycle = 0.f;
    pid_right.integral = 0.f;
    pid_right.prev_error = 0.f;
    pid_right.last_time = pid_left.last_time;
}

#endif