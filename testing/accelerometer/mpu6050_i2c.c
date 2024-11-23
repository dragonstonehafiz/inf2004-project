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

    // Main loop
    while (1) 
    {
        static float prev_forward_percentage = 0.0f;  // Changed from prev_forward_speed
        static float prev_turn_speed = 0.0f;
        static char prev_forward_dir = 'N';
        static char prev_turn_dir = 'N';

        // Initialize movement_data_t
        movement_data_t movement_data = {'N', 0.0f, 'N', 0.0f};

        float pitch, roll, yaw;
        char* data_to_send;

        // Read accelerometer data
        calculate_angles(&pitch, &roll, &yaw);
        
        // Process movement commands
        movement_data = get_command(pitch, roll, movement_data);

        // Check if current data is different from previous data
        if (movement_data.forward_direction != prev_forward_dir ||
            movement_data.forward_percentage != prev_forward_percentage ||  // Changed from prev_forward_speed
            movement_data.turn_direction != prev_turn_dir ||
            movement_data.turn_percentage != prev_turn_speed) {
            
            // Data has changed, send it
            data_to_send = serialize_movement_data(&movement_data);
            // printf("%s", data_to_send);

            // Update previous values
            prev_forward_dir = movement_data.forward_direction;
            prev_forward_percentage = movement_data.forward_percentage;  // Changed from prev_forward_speed
            prev_turn_dir = movement_data.turn_direction;
            prev_turn_speed = movement_data.turn_percentage;
        }

        sleep_ms(100);  // Shorter delay for more responsiveness
    }
#endif
}
#endif  // Ending the i2c_default block
