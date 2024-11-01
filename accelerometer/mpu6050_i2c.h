#ifndef LSM303DLHC_H
#define LSM303DLHC_H

#include <stdint.h>
#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "pico/binary_info.h"
#include "pico/stdlib.h"

// Thresholds for movements
#define FORWARD_BACKWARD_THRESHOLD 10.0  // Threshold for forward/backward movement
#define TURN_THRESHOLD 5.0               // Threshold for turning
#define MAX_SPEED 100                    // Max speed percentage

// LSM303DLHC I2C addresses
#define LSM303_ADDR_ACCEL 0x19 // Accelerometer address
#define LSM303_ADDR_MAG 0x1E   // Magnetometer address

// Register addresses for LSM303DLHC
#define LSM303_REG_ACCEL_CTRL_REG1_A 0x20 // Control register for accelerometer
#define LSM303_REG_ACCEL_OUT_X_L_A 0x28   // Output register for accelerometer X-axis LSB
#define LSM303_REG_MAG_CRA_REG_M 0x00     // Control register A for magnetometer
#define LSM303_REG_MAG_OUT_X_H_M 0x03     // Output register for magnetometer X-axis MSB

// Scale factors
#define ACCEL_SCALE 0.001       // Accelerometer scale for ±2g range (1 mg/LSB)
#define MAG_SCALE (1.0 / 1100.0)  // Magnetometer scale for ±1.3 gauss range (1100 LSB/gauss)

// Low-pass filter parameters for stability
#define ALPHA 0.5  // Filter smoothing factor

// Global variables for filtered accelerometer data
extern float filtered_accel[3];

// Function prototypes
void lsm303dlhc_init(void);
void i2c_scan(void);
void lsm303dlhc_read_filtered_accel(int16_t accel[3]);
void lsm303dlhc_read_raw_mag(int16_t mag[3]);
void calculate_angles(int16_t accel[3], float* pitch, float* roll);
void generate_forward_backward_command(float pitch);
void generate_turn_command(float roll);

#endif // LSM303DLHC_H
