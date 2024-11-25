#include "states.h"
#include "wheels.h"
#include "network.h"
#include "ultrasonic.h"
#include "ir_sensor.h"
#include "encoder.h"

#define RECIEVER_UDP_PORT 4444
#define MESSAGE_BUFFER_SIZE 128

volatile uint8_t currState;
// set to true if ultrasonic sees something that is <10cm
volatile bool tooClose = false;
volatile uint64_t ultrasonicLastCheckTime;
float lastUltrasonicDistance;
// Last time a message was sent to the dashboard
uint64_t lastMessageSentTime = 1;

void handleControls(movement_data_t *movementData);
void handleUltrasonic();

void changeState(uint8_t nextState)
{
    currState = nextState;
    send_udp_data("CHANGE STATE\n", PORT_DASHBOARD, IP_DASHBOARD);
    switch (nextState)
    {
    case STATE_INITIAL:
        break;
    case STATE_CONNECTING:
        break;
    case STATE_REMOTE:
        break;
    case STATE_AUTO:
        set_car_state(CAR_FORWARD);
        set_wheels_duty_cycle(0.5f);
        break;
    case STATE_END:
        deinit_server();
        break;
    default:
        printf("ヤバイ\n");
        break;
    }
}

uint8_t getState()
{
    return currState;
}

void updateCore0()
{
    while (true)
    {
        movement_data_t *movementData = NULL;

        switch (currState)
        {
        case STATE_INITIAL:
            break;
        case STATE_CONNECTING:
            if (connect_to_wifi())
            {
                changeState(STATE_REMOTE);
                init_udp_server_reciever(RECIEVER_UDP_PORT);
                init_udp_server_sender(IP_DASHBOARD);
            }
            else
            {
                printf("Connection failed. Press button 21 to try again.\n");
                changeState(STATE_INITIAL);
            }
            break;
        case STATE_REMOTE:
            movementData = get_movement_data();
            handleUltrasonic();
            handleControls(movementData);
            break;
        case STATE_AUTO:
            handle_barcode();
            handle_line_tracing(&set_car_state);
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
    char message[MESSAGE_BUFFER_SIZE]; // Character buffer for the message
    while (true)
    {
        uint64_t now = time_us_64();
        // Send a message every 1 microsecond
        if (now - lastMessageSentTime > 1000000)
        {
            switch (currState)
            {
            case STATE_INITIAL:
            case STATE_CONNECTING:
                break;
            case STATE_REMOTE:
                snprintf(message, MESSAGE_BUFFER_SIZE, "Ultrasonic Distance: %.2f", lastUltrasonicDistance);
                send_udp_data(message, PORT_DASHBOARD, IP_DASHBOARD);
                snprintf(message, MESSAGE_BUFFER_SIZE, "Left Wheel Distance: %.2f, Right Wheel Distance", leftTotalDistance, rightTotalDistance);
                send_udp_data(message, PORT_DASHBOARD, IP_DASHBOARD);
                break;
            case STATE_AUTO:
                snprintf(message, MESSAGE_BUFFER_SIZE, "Ultrasonic Distance: %.2f", lastUltrasonicDistance);
                send_udp_data(message, PORT_DASHBOARD, IP_DASHBOARD);
                break;
            default:
                break;
            }
            lastMessageSentTime = now;
        }
    }
}

void handleControls(movement_data_t *movementData)
{
    if (movementData == NULL)
        return;
    else
    {
        // starting duty cycle not representative of duty cycle required to move
        float minimumDutyCycle = 0.55f;
        float remainder = 0.45f;
        // ははは この授業は凄く楽しいです
        // 皮肉じゃないよ
        // You can only move forward/backward or turn.
        // You cannot turn while going forward/backward. Why? Because I give up, that's why.
        if (movementData->forward_direction == 'F')
        {
            if (!tooClose)
            {
                set_car_state(CAR_FORWARD);
                float dutyCycle = minimumDutyCycle + remainder * movementData->forward_percentage;
                set_wheels_duty_cycle(dutyCycle);
                pid_right.enabled = true;
            }
            else
            {
                set_car_state(CAR_STATIONARY);
                pid_right.enabled = false;
            }
        }
        else if (movementData->forward_direction == 'B')
        {
            set_car_state(CAR_BACKWARD);
            float dutyCycle = minimumDutyCycle + remainder * movementData->forward_percentage;
            set_wheels_duty_cycle(dutyCycle);
            pid_right.enabled = true;
        }
        else if (movementData->forward_direction == 'T')
        {
            changeState(STATE_AUTO);
        }
        else
        {
            // We won't need pid when turning
            pid_right.enabled = false;
            reset_pid();
            if (movementData->turn_direction == 'R' && !tooClose)
                set_car_state(CAR_TURN_RIGHT);
            else if (movementData->turn_direction == 'L' && !tooClose)
                set_car_state(CAR_TURN_LEFT);
            else
                set_car_state(CAR_STATIONARY);
            set_wheels_duty_cycle(movementData->turn_percentage);
        }
    }
}
void handleUltrasonic()
{
    uint64_t currentTime = time_us_64();
    uint64_t timeDiff = currentTime - ultrasonicLastCheckTime;

    // timediff larger than 100ms
    if (timeDiff > 100000)
    {
        triggerPulse();

        sleep_ms(30);

        lastUltrasonicDistance = getCm();
        // printf("calculatedDist: %.2f\n", calculatedDist);
        if (lastUltrasonicDistance <= 25.f && lastUltrasonicDistance >= 0.f)
            tooClose = true;
        else
            tooClose = false;

        ultrasonicLastCheckTime = currentTime;
    }
}
