#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/i2c.h"
#include "accelerometer.h"
#include <math.h>

#ifdef i2c_default  // Check if i2c_default is defined
int main() 
{
    stdio_init_all();
    init_accelerometer();

#if !defined(i2c_default) || !defined(PICO_DEFAULT_I2C_SDA_PIN) || !defined(PICO_DEFAULT_I2C_SCL_PIN)
    #warning i2c/lsm303dlhc example requires a board with I2C pins
    puts("Default I2C pins were not defined");
    return 0;
#else
    printf("Hello, LSM303DLHC! Reading raw and adjusted data from registers...\n");

    float pitch, roll, yaw;

    // Main loop
    while (1) 
    {
        // Calculate the angles based on accelerometer and magnetometer data
        calculate_angles(&pitch, &roll, &yaw);
        
        bool moveFront, turnRight;
        float scalarFront, scalarRight;

        get_command_forward(pitch, &moveFront, &turnRight);
        get_command_turn(roll, &turnRight, &scalarRight);

        // Print raw pitch and roll values
        // printf("Raw Pitch = %.2f°, Raw Roll = %.2f°\n", pitch * 180.0 / M_PI, roll * 180.0 / M_PI);

        sleep_ms(500);  // Shorter delay for more responsiveness
    }
#endif
}
#endif  // Ending the i2c_default block
