#ifndef FINAL_H_
#define FINAL_H_

#include "pico/stdlib.h"

extern uint8_t currState;

enum STATES_FINAL
{
    STATE_INITIAL = 0,
    STATE_CONNECTING,
    STATE_REMOTE,
    STATE_AUTO,
    STATE_END,
    STATE_SERVER_ERROR,
    
    STATE_NUM
};

void changeState(uint8_t nextState);
void updateCore0();
void updateCore1();

#endif