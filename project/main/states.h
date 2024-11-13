#ifndef FINAL_H_
#define FINAL_H_

#include "pico/stdlib.h"

enum PROGRAM_STATES
{
    STATE_IDLE = 0,
    STATE_REMOTE,
    STATE_AUTO,
    
    STATE_NUM
};

uint8_t currState = STATE_IDLE;

#endif