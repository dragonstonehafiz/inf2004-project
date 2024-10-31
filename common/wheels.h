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

    NUM_CAR_STATES
};

uint8_t carState = CAR_STATIONARY;

void init_wheels();
void set_car_state(uint8_t nextState);

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
            break;
        case CAR_FORWARD:
            // When moving forward, right wheel is clockwise, left wheel is counter clockwise
            set_motor_direction(WHEEL_LEFT_OUT_PIN_1, WHEEL_LEFT_OUT_PIN_2, false);
            set_motor_direction(WHEEL_RIGHT_OUT_PIN_1, WHEEL_RIGHT_OUT_PIN_2, true);
            break;
        case CAR_BACKWARD:
            // When moving forward, right wheel is counter clockwise, left wheel is clockwise
            set_motor_direction(WHEEL_LEFT_OUT_PIN_1, WHEEL_LEFT_OUT_PIN_2, true);
            set_motor_direction(WHEEL_RIGHT_OUT_PIN_1, WHEEL_RIGHT_OUT_PIN_2, false);
            break;
        case CAR_TURN_RIGHT:
            // When turning right, right wheel is stationary, left wheel is counter clockwise
            set_motor_stop(WHEEL_RIGHT_OUT_PIN_1, WHEEL_RIGHT_OUT_PIN_2);
            set_motor_direction(WHEEL_LEFT_OUT_PIN_1, WHEEL_LEFT_OUT_PIN_2, false);
            break;
        case CAR_TURN_LEFT:
            // When turning right, left wheel is stationary, right wheel is clockwise
            set_motor_stop(WHEEL_LEFT_OUT_PIN_1, WHEEL_LEFT_OUT_PIN_2);
            set_motor_direction(WHEEL_RIGHT_OUT_PIN_1, WHEEL_RIGHT_OUT_PIN_2, true);
            break;
        default:
            break;
    }
}


#endif