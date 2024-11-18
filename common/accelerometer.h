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


typedef struct {
    char forward_direction;
    float forward_percentage;
    char turn_direction;
    float turn_percentage;  // Changed from turn_speed to match the struct usage
} movement_data_t;


// Function declarations
static void i2c_scan(void);
void init_accelerometer(void);
static void lsm303dlhc_read_raw(int16_t accel[3], int16_t mag[3]);
void calculate_angles(float* pitch, float* roll, float* yaw);
float convert_to_discrete_percentage(float input_percentage);
movement_data_t get_command(float pitch, float roll, movement_data_t movement);
char* serialize_movement_data(const movement_data_t* data);
void get_data_to_send(void);


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

// Get forward and turn command at once
movement_data_t get_command(float pitch, float roll, movement_data_t movement) 
{
    // Check if within ±30 degrees threshold
    if (pitch <= FORWARD_BACKWARD_THRESHOLD && pitch >= -FORWARD_BACKWARD_THRESHOLD) 
    {
        // Within threshold - set to stop
        movement.forward_direction = 'N';
        movement.forward_percentage = 0.0f;  // Changed from speed_percentage
    }
    else if (pitch > FORWARD_BACKWARD_THRESHOLD) 
    {
        // Moving forward
        movement.forward_direction = 'F';
        movement.forward_percentage = convert_to_discrete_percentage((pitch / M_PI_2) * MAX_SPEED);
        if (movement.forward_percentage > MAX_SPEED) 
            movement.forward_percentage = MAX_SPEED;
    }
    else if (pitch < -FORWARD_BACKWARD_THRESHOLD) 
    {
        // Moving backward
        movement.forward_direction = 'B';
        movement.forward_percentage = convert_to_discrete_percentage((-pitch / M_PI_2) * MAX_SPEED);
        if (movement.forward_percentage > MAX_SPEED) 
            movement.forward_percentage = MAX_SPEED;
    }

    // Turn commands remain unchanged
    if (roll <= TURN_THRESHOLD && roll >= -TURN_THRESHOLD) 
    {
        movement.turn_direction = 'N';
        movement.turn_percentage = 0.0f;
    }
    else if (roll > TURN_THRESHOLD) 
    {
        movement.turn_direction = 'R';
        movement.turn_percentage = convert_to_discrete_percentage((roll / M_PI_2) * MAX_SPEED);
        if (movement.turn_percentage > MAX_SPEED) 
            movement.turn_percentage = MAX_SPEED;
    }
    else if (roll < -TURN_THRESHOLD) 
    {
        movement.turn_direction = 'L';
        movement.turn_percentage = convert_to_discrete_percentage((-roll / M_PI_2) * MAX_SPEED);
        if (movement.turn_percentage > MAX_SPEED) 
            movement.turn_percentage = MAX_SPEED;
    }

    return movement;
}


// Function to serialize movement data
char* serialize_movement_data(const movement_data_t* data) {
    static char buffer[MAX_DATA_STRING];
    snprintf(buffer, sizeof(buffer), "PICO|%c:%.1f,%c:%.1f\n",
        data->forward_direction,
        data->forward_percentage,  // Changed from speed_percentage
        data->turn_direction,
        data->turn_percentage
    );
    return buffer;
}

void get_data_to_send(void) {
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
        send_udp_data(data_to_send);

        // Update previous values
        prev_forward_dir = movement_data.forward_direction;
        prev_forward_percentage = movement_data.forward_percentage;  // Changed from prev_forward_speed
        prev_turn_dir = movement_data.turn_direction;
        prev_turn_speed = movement_data.turn_percentage;
    }
}