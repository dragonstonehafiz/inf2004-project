#include "pico/multicore.h"
#include <string.h>

#include "states.h"
#include "setup.h"
#include "reciever.h"


int main() 
{
    init_gpio();
    init_interrupts();
    init_server();
    changeState(STATE_INITIAL);

    multicore_launch_core1(updateCore1);
    updateCore0();

    deinit_server();
    return 0;
}