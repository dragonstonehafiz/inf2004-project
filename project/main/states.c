#include "states.h"
#include "reciever.h"

uint8_t currState;
extern uint8_t running;

void changeState(uint8_t nextState)
{
    currState = nextState;
    switch (nextState)
    {
        case STATE_INITIAL:
            break;
        case STATE_CONNECTING:
            init_server();
            break;
        case STATE_REMOTE:
            break;
        case STATE_AUTO:
            break;
        case STATE_END:
            deinit_server();
            running = 0;
            break;
        default:
            printf("ヤバイ\n");
            break;
    }
}
void updateCore0()
{
    switch (currState)
    {
        case STATE_INITIAL:
            break;
        case STATE_CONNECTING:
            connect_to_wifi();
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