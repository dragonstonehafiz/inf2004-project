#include <string.h>

#include "states.h"
#include "setup.h"

uint8_t running = 1;

int main() 
{
    init_gpio();
    init_interrupts();
    changeState(STATE_INITIAL);

    while (running == 1)
        updateCore0();

    return 0;
}