#include "states.h"
#include "reciever.h"

uint8_t currState;
extern bool running;

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
            print_movement_data(movementData);
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