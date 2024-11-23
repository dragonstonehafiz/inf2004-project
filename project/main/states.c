#include "states.h"
#include "reciever.h"
#include "wheels.h"
#include "ultrasonic.h"

uint8_t currState;
// set to true if ultrasonic sees something that is <10cm
bool tooClose = false;
uint64_t ultrasonicLastCheckTime;

void handleControls(movement_data_t *movementData);
void handleUltrasonic()
{
    uint64_t currentTime = time_us_64();
    uint64_t timeDiff = currentTime - ultrasonicLastCheckTime;

    // timediff larger than 100ms
    if (timeDiff > 100000)
    {
        triggerPulse();
        sleep_ms(30);

        float calculatedDist = getCm();
        if (calculatedDist <= 15.f && calculatedDist >= 0.f)
            tooClose = true;
        else
            tooClose = false;
        
        ultrasonicLastCheckTime = currentTime;
    }
}

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
            break;
        default:
            printf("ヤバイ\n");
            break;
    }
}
void updateCore0()
{
    while (true)
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
}
void updateCore1()
{
    while (true)
    {
        switch (currState)
        {
            case STATE_INITIAL:
                break;
            case STATE_CONNECTING:
                break;
            case STATE_REMOTE:
                handleUltrasonic();
                break;
            case STATE_AUTO:
                handleUltrasonic();
                break;
            case STATE_END:
                break;
            default:
                printf("ヤバイ\n");
                break;
        }
    }
}

void handleControls(movement_data_t *movementData)
{
    if (movementData == NULL)
        return;
    else if (tooClose)
    {
        set_car_state(CAR_STATIONARY);
        reset_pid();
    }
    else
    {
        // ははは この授業は凄く楽しいです
        // 皮肉じゃないよ
        // You can only move forward/backward or turn.
        // You cannot turn while going forward/backward. Why? Because I give up, that's why.
        if (movementData->forward_direction == 'F')
        {
            set_car_state(CAR_FORWARD);
            set_left_wheel_duty_cycle(movementData->forward_percentage);
            pid_right.enabled = true;
        }
        else if (movementData->forward_direction == 'B')
        {
            set_car_state(CAR_BACKWARD);
            set_left_wheel_duty_cycle(movementData->forward_percentage);
            pid_right.enabled = true;
        }
        else
        {
            // We won't need pid when turning
            pid_right.enabled = false;
            reset_pid();
            set_wheels_duty_cycle(movementData->forward_percentage);
            if (movementData->turn_direction == 'R')
                set_car_state(CAR_TURN_RIGHT);
            else if (movementData->turn_direction == 'L')
                set_car_state(CAR_TURN_LEFT);
            else
                set_car_state(CAR_STATIONARY);
        }
    }
}