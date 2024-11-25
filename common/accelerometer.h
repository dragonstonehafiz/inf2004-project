#ifndef ACCELEROMETER_H_
#define ACCELEROMETER_H_

#include <stdio.h>

typedef struct {
    char forward_direction;
    float forward_percentage;
    char turn_direction;
    float turn_percentage;
} data_to_send_t;

// Function declarations
static void i2c_scan(void);
void init_accelerometer(void);
static void lsm303dlhc_read_raw(int16_t accel[3], int16_t mag[3]);
void calculate_angles(float* pitch, float* roll, float* yaw);
float convert_to_discrete_percentage_forward(float pitch);
float convert_to_discrete_percentage_turning(float roll);
data_to_send_t get_command(float pitch, float roll, data_to_send_t movement);
char* serialize_movement_data(const data_to_send_t* data);
char* get_data_to_send(void);

#endif