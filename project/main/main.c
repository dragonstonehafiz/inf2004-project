#include <string.h>

#include "states.h"
#include "setup.h"
#include "reciever.h"

bool running = true;

int main() 
{
    init_gpio();
    init_interrupts();
    init_server();
    changeState(STATE_INITIAL);

    while (true)
        updateCore0();

    deinit_server();
    return 0;
}