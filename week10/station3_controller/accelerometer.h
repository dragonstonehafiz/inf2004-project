#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/i2c.h"
#include <math.h>
#include "udp.h"

// LSM303DLHC I2C addresses
#define LSM303_ADDR_ACCEL 0x19 // Accelerometer address
#define LSM303_ADDR_MAG 0x1E   // Magnetometer address

// Register addresses for LSM303DLHC
#define LSM303_REG_ACCEL_CTRL_REG1_A 0x20 // Control register for accelerometer
#define LSM303_REG_ACCEL_OUT_X_L_A 0x28   // Output register for accelerometer X-axis LSB
#define LSM303_REG_MAG_CRA_REG_M 0x00     // Control register A for magnetometer
#define LSM303_REG_MAG_OUT_X_H_M 0x03     // Output register for magnetometer X-axis MSB


#define MAX_DATA_STRING 32

// Scale factors
#define ACCEL_SCALE 0.001       // Accelerometer scale for ±2g range (1 mg/LSB)
#define MAG_SCALE (1.0 / 1100.0)  // Magnetometer scale for ±1.3 gauss range (1100 LSB/gauss)

// Thresholds for movements
#define FORWARD_BACKWARD_THRESHOLD (15.0/180.0 * M_PI)  // Threshold for forward/backward movement
#define TURN_THRESHOLD (15.0/180.0 * M_PI)               // Threshold for turning
#define MAX_SPEED 100                    // Max speed percentage

// Direction enums for clearer return values
typedef enum {
    DIRECTION_NONE,
    DIRECTION_FORWARD,
    DIRECTION_BACKWARD
} movement_direction_t;

typedef enum {
    TURN_NONE,
    TURN_LEFT,
    TURN_RIGHT
} turn_direction_t;

// Struct to hold movement commands
typedef struct {
    movement_direction_t direction;
    float speed_percentage;
} movement_command_t;
 
typedef struct {
    turn_direction_t direction;
    float turn_percentage;
} turn_command_t;


typedef struct {
    movement_direction_t forward_direction;
    float forward_speed;
    turn_direction_t turn_direction;
    float turn_speed;
    uint32_t timestamp;
} movement_data_t;


// Function to scan I2C bus for connected devices
static void i2c_scan() 
{
    for (int addr = 0; addr < 127; addr++) 
    {
        if (i2c_write_blocking(i2c_default, addr, NULL, 0, false) >= 0) 
            printf("Found I2C device at address 0x%02x\n", addr);
    }
}

// Function to initialize the LSM303DLHC accelerometer and magnetometer
void init_accelerometer() 
{
    uint8_t buf[2];

     // Initialize I2C at 400kHz
    i2c_init(i2c_default, 400 * 1000);
    gpio_set_function(PICO_DEFAULT_I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(PICO_DEFAULT_I2C_SDA_PIN);
    gpio_pull_up(PICO_DEFAULT_I2C_SCL_PIN);
    bi_decl(bi_2pins_with_func(PICO_DEFAULT_I2C_SDA_PIN, PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C));

    // Initialize accelerometer: 100Hz, all axes enabled
    buf[0] = LSM303_REG_ACCEL_CTRL_REG1_A;
    buf[1] = 0x57; // 0b01010111: 100Hz, all axes enabled
    if (i2c_write_blocking(i2c_default, LSM303_ADDR_ACCEL, buf, 2, false) < 0)
        printf("Failed to initialize accelerometer\n");

    // Initialize magnetometer: Continuous conversion mode, 220 Hz
    buf[0] = LSM303_REG_MAG_CRA_REG_M;
    buf[1] = 0x1C; // 0b00011100: Continuous conversion, 220 Hz
    if (i2c_write_blocking(i2c_default, LSM303_ADDR_MAG, buf, 2, false) < 0)
        printf("Failed to initialize magnetometer\n");

    // Scan for I2C devices
    i2c_scan();
}


static void lsm303dlhc_read_raw(int16_t accel[3], int16_t mag[3]) 
{
    uint8_t buffer[6];
    uint8_t reg_addr[1];

    // Read accelerometer data
    reg_addr[0] = LSM303_REG_ACCEL_OUT_X_L_A | 0x80;
    if (i2c_write_blocking(i2c_default, LSM303_ADDR_ACCEL, reg_addr, 1, true) < 0) 
    {
        printf("Failed to write to accelerometer\n");
        return;
    }
    if (i2c_read_blocking(i2c_default, LSM303_ADDR_ACCEL, buffer, 6, false) < 0) 
    {
        printf("Failed to read accelerometer data\n");
        return;
    }
    // Combine high and low bytes for accelerometer data
    for (int i = 0; i < 3; i++) 
    {
        accel[i] = (int16_t)((buffer[2 * i + 1] << 8) | buffer[2 * i]); // Combine high and low byte
    }

    // Read magnetometer data
    reg_addr[0] = LSM303_REG_MAG_OUT_X_H_M;
    if (i2c_write_blocking(i2c_default, LSM303_ADDR_MAG, reg_addr, 1, true) < 0) 
    {
        printf("Failed to write to magnetometer\n");
        return;
    }
    if (i2c_read_blocking(i2c_default, LSM303_ADDR_MAG, buffer, 6, false) < 0) 
    {
        printf("Failed to read magnetometer data\n");
        return;
    }
    
    // Combine high and low bytes for magnetometer data
    for (int i = 0; i < 3; i++) 
        mag[i] = (int16_t)((buffer[2 * i] << 8) | buffer[2 * i + 1]); 
}

// Function to calculate pitch, roll, and yaw
void calculate_angles(float* pitch, float* roll, float* yaw) 
{
    // Get raw input from accelerometer
    static int16_t accel[3], mag[3];
    lsm303dlhc_read_raw(accel, mag);

    // Convert accelerometer data to g's
    float ax = accel[0] * ACCEL_SCALE;
    float ay = accel[1] * ACCEL_SCALE;
    float az = accel[2] * ACCEL_SCALE;
    float azaz = az * az;

    // Calculate pitch and roll
    *pitch = atan2(ay, sqrt(ax * ax + azaz));  // Pitch in radians
    *roll = atan2(ax, sqrt(ay * ay + azaz));   // Roll in radians

    // Convert magnetometer data to gauss
    float mx = mag[0] * MAG_SCALE;
    float my = mag[1] * MAG_SCALE;
    float mz = mag[2] * MAG_SCALE;

    // Adjust magnetometer readings by pitch and roll
    float mx2 = mx * cos(*pitch) + mz * sin(*pitch);
    float my2 = mx * sin(*roll) * sin(*pitch) + my * cos(*roll) - mz * sin(*roll) * cos(*pitch);

    // Calculate yaw (compass heading)
    *yaw = atan2(my2, mx2);  // Yaw in radians
}

float convert_to_discrete_percentage(float input_percentage) 
{
    if (input_percentage <= 15.0f) {
        return 0.0f;
    }
    else if (input_percentage > 15.0f && input_percentage <= 30.0f) {
        return 20.0f;
    }
    else if (input_percentage > 30.0f && input_percentage <= 45.0f) {
        return 40.0f;
    }
    else if (input_percentage > 45.0f && input_percentage <= 60.0f) {
        return 60.0f;
    }
    else if (input_percentage > 60.0f && input_percentage <= 75.0f) {
        return 80.0f;
    }
    else if (input_percentage > 75.0f && input_percentage <= 90.0f) {
        return 100.0f;
    }
    else {  // > 90.0f
        return 100.0f;
    }
}

movement_command_t get_command_forward(float pitch) 
{
    movement_command_t command = {DIRECTION_NONE, 0.0f};
    
    // Debug print to see actual pitch value
    // printf("Current pitch (radians): %f, Threshold: %f\n", pitch, FORWARD_BACKWARD_THRESHOLD);
    
    // Check if within ±30 degrees threshold
    if (pitch <= FORWARD_BACKWARD_THRESHOLD && pitch >= -FORWARD_BACKWARD_THRESHOLD) 
    {
        // Within threshold - set to stop
        command.direction = DIRECTION_NONE;
        command.speed_percentage = 0.0f;
    }
    else if (pitch > FORWARD_BACKWARD_THRESHOLD) 
    {
        // Moving forward
        command.direction = DIRECTION_FORWARD;
        command.speed_percentage = convert_to_discrete_percentage((pitch / M_PI_2) * MAX_SPEED);
        if (command.speed_percentage > MAX_SPEED) 
            command.speed_percentage = MAX_SPEED;
    }
    else if (pitch < -FORWARD_BACKWARD_THRESHOLD) 
    {
        // Moving backward
        command.direction = DIRECTION_BACKWARD;
        command.speed_percentage = convert_to_discrete_percentage((-pitch / M_PI_2) * MAX_SPEED);
        if (command.speed_percentage > MAX_SPEED) 
            command.speed_percentage = MAX_SPEED;
    }
    
    return command;
}

turn_command_t get_command_turn(float roll) 
{
    turn_command_t command = {TURN_NONE, 0.0f};
    
    // Check if within ±30 degrees threshold
    if (roll <= TURN_THRESHOLD && roll >= -TURN_THRESHOLD) 
    {
        // Within threshold - set to stop
        command.direction = TURN_NONE;  // Note: Changed from DIRECTION_NONE to TURN_NONE
        command.turn_percentage = 0.0f;
    }
    // Check for turning left or right based on roll angle
    else if (roll > TURN_THRESHOLD) 
    {
        // Turning right
        command.direction = TURN_RIGHT;
        command.turn_percentage = convert_to_discrete_percentage((roll / M_PI_2) * MAX_SPEED);
        if (command.turn_percentage > MAX_SPEED) 
            command.turn_percentage = MAX_SPEED;
    }
    else if (roll < -TURN_THRESHOLD) 
    {
        // Turning left
        command.direction = TURN_LEFT;
        command.turn_percentage = convert_to_discrete_percentage((-roll / M_PI_2) * MAX_SPEED);
        if (command.turn_percentage > MAX_SPEED) 
            command.turn_percentage = MAX_SPEED;
    }
    
    // Debug output
    // if (command.direction != TURN_NONE) {
    //     printf("Command: Turn %s at %.1f%% speed\n", 
    //            command.direction == TURN_RIGHT ? "right" : "left",
    //            command.turn_percentage);
    // } else {
    //     printf("Command: Stop turning\n");
    // }
    
    return command;
}



char get_movement_direction_char(movement_direction_t forward_direction) {
    switch(forward_direction) {
        case DIRECTION_FORWARD:  return 'F';
        case DIRECTION_BACKWARD: return 'B';
        case DIRECTION_NONE:     return 'N';
        default:                 return 'N';
    }
}

char get_turn_direction_char(turn_direction_t turn_direction) {
    switch(turn_direction) {
        case TURN_LEFT:  return 'L';
        case TURN_RIGHT: return 'R';
        case TURN_NONE:  return 'N';
        default:         return 'N';
    }
}

// char* serialize_movement_data(const movement_data_t* data) {
//     static char buffer[MAX_DATA_STRING];
//     snprintf(buffer, sizeof(buffer), "%lu,%c,%.1f,%c,%.1f\n",
//         data->timestamp,
//         get_movement_direction_char(data->forward_direction),
//         data->forward_speed,
//         get_turn_direction_char(data->turn_direction),
//         data->turn_speed
//     );
//     return buffer;
// }

char* serialize_movement_data(const movement_data_t* data) {
    static char buffer[MAX_DATA_STRING];
    snprintf(buffer, sizeof(buffer), "PICO|%c:%.1f,%c:%.1f\n",
        get_movement_direction_char(data->forward_direction),
        data->forward_speed,
        get_turn_direction_char(data->turn_direction),
        data->turn_speed
    );
    return buffer;
}

void get_data_to_send() {
    static float prev_forward_speed = 0.0f;
    static float prev_turn_speed = 0.0f;
    static movement_direction_t prev_forward_dir = DIRECTION_NONE;
    static turn_direction_t prev_turn_dir = TURN_NONE;
    
    float pitch, roll, yaw;
    movement_data_t movement_data;
    char* data_to_send;

    // Read accelerometer data
    calculate_angles(&pitch, &roll, &yaw);
    
    // Process movement commands
    movement_command_t forward_command = get_command_forward(pitch);
    turn_command_t turn_command = get_command_turn(roll);

    // Update movement data structure
    movement_data.forward_direction = forward_command.direction;
    movement_data.forward_speed = forward_command.speed_percentage;
    movement_data.turn_direction = turn_command.direction;
    movement_data.turn_speed = turn_command.turn_percentage;
    movement_data.timestamp = to_ms_since_boot(get_absolute_time());

    // Check if current data is different from previous data
    if (movement_data.forward_direction != prev_forward_dir ||
        movement_data.forward_speed != prev_forward_speed ||
        movement_data.turn_direction != prev_turn_dir ||
        movement_data.turn_speed != prev_turn_speed) {
        
        // Data has changed, send it
        data_to_send = serialize_movement_data(&movement_data);
        send_udp_data(data_to_send);

        // Update previous values
        prev_forward_dir = movement_data.forward_direction;
        prev_forward_speed = movement_data.forward_speed;
        prev_turn_dir = movement_data.turn_direction;
        prev_turn_speed = movement_data.turn_speed;
    }
}