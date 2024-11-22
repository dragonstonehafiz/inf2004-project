#include "states.h"
#include "reciever.h"
#include "wheels.h"

uint8_t currState;
extern bool running;

void handleControls(movement_data_t *movementData);

void changeState(uint8_t nextState)
{
    currState = nextState;
    switch (nextState)
    {
        case STATE_INITIAL:
            break;
        case STATE_CONNECTING:
            break;
        case STATE_REMOTE:
            break;
        case STATE_AUTO:
            break;
        case STATE_END:
            deinit_server();
            running = false;
            break;
        default:
            printf("ヤバイ\n");
            break;
    }
}
void updateCore0()
{
    movement_data_t * movementData = NULL; 

    switch (currState)
    {
        case STATE_INITIAL:
            break;
        case STATE_CONNECTING:
            connect_to_wifi();
            break;
        case STATE_REMOTE:
            movementData = get_movement_data();
            handleControls(movementData);
            printf("leftDutyCycle: %.2f, rightDutyCycle: %.2f\n", pid_left.duty_cycle, pid_right.duty_cycle);
            break;
        case STATE_AUTO:
            break;
        case STATE_END:
            break;
        default:
            printf("ヤバイ\n");
            break;
    }
}
void updateCore1()
{
    switch (currState)
    {
        case STATE_INITIAL:
            break;
        case STATE_CONNECTING:
            break;
        case STATE_REMOTE:
            break;
        case STATE_AUTO:
            break;
        case STATE_END:
            break;
        default:
            printf("ヤバイ\n");
            break;
    }
}

void handleControls(movement_data_t *movementData)
{
    if (movementData == NULL)
        return;
    else
    {
        // ははは この授業は凄く楽しいです
        // 皮肉じゃないよ
        // Handle forward direction
        if (movementData->forward_direction == 'F')
        {
            set_car_state(CAR_FORWARD);
            
            // If we are moving forward and turning right,
            // left wheel should be slower than right wheel
            // 実はさ、僕の部屋は狭すぎてコードをテストするのは難しい
            if (movementData->turn_direction == 'R')
            {
                float *leftDutyCycle = &movementData->forward_percentage;
                set_left_wheel_duty_cycle(*leftDutyCycle);
                float rightDutyCycle = (*leftDutyCycle * (1 - movementData->turn_percentage));
                set_right_wheel_duty_cycle(rightDutyCycle);
            }
            // If turn left, right wheel should be slower than right
            else if (movementData->turn_direction == 'L')
            {
                float *rightDutyCycle = &movementData->forward_percentage;
                set_right_wheel_duty_cycle(*rightDutyCycle);
                float leftDutyCycle = (*rightDutyCycle * (1 - movementData->turn_percentage));
                set_left_wheel_duty_cycle(leftDutyCycle);
            }
            // If moving forward, just make right wheel match left wheel pid
            else 
            {
                float *leftDutyCycle = &movementData->forward_percentage;
                set_left_wheel_duty_cycle(*leftDutyCycle);
                pid_right.enabled = true;
            }
        }
        // For backward movement, turning not handled
        else if (movementData->forward_direction == 'B')
        {
            set_car_state(CAR_BACKWARD);
            float *leftDutyCycle = &movementData->forward_percentage;
            set_left_wheel_duty_cycle(*leftDutyCycle);
            pid_right.enabled = true;
        }
        else
        {
            pid_right.enabled = false;
            set_car_state(CAR_STATIONARY);
        }
    }
}