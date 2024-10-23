#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include <math.h>
#include <string.h>

#define LSM303DLHC_MAG_ADDR 0x1E // Magnetometer I2C address for LSM303DLHC
#define LSM303DLHC_ACC_ADDR 0x19 // Accelerometer I2C address for LSM303DLHC

// Magnetometer register addresses
#define CRA_REG_M 0x00
#define CRB_REG_M 0x01
#define MR_REG_M  0x02
#define OUT_X_H_M 0x03
#define OUT_X_L_M 0x04
#define OUT_Y_H_M 0x07
#define OUT_Y_L_M 0x08
#define OUT_Z_H_M 0x05
#define OUT_Z_L_M 0x06

// Accelerometer register addresses
#define CTRL_REG1_A 0x20
#define OUT_X_L_A 0x28
#define OUT_X_H_A 0x29
#define OUT_Y_L_A 0x2A
#define OUT_Y_H_A 0x2B
#define OUT_Z_L_A 0x2C
#define OUT_Z_H_A 0x2D

// Scale factors for converting raw data to physical units
#define SCALE_FACTOR_MAG 0.92  // Scale factor for magnetometer data (uT)
#define SCALE_FACTOR_ACC 0.001 // Scale factor for accelerometer data (g)

int16_t read16(uint8_t addr, uint8_t device_addr) {
    uint8_t data[2];
    if (i2c_write_blocking(i2c0, device_addr, &addr, 1, true) < 0) {
        printf("I2C write failed for address 0x%02x\n", device_addr);
        return -1;
    }
    if (i2c_read_blocking(i2c0, device_addr, data, 2, false) < 0) {
        printf("I2C read failed for address 0x%02x\n", device_addr);
        return -1;
    }
    return (data[0] << 8) | data[1];
}

// Calculate the heading based on magnetometer's X and Y axes
void wait_and_set_direction(const char* direction, int delay_ms) {
    printf("Position the robot facing %s and wait...\n", direction);
    sleep_ms(delay_ms);

    int16_t mag_x, mag_y, mag_z;
    double headingSum = 0;
    int numReadings = 15; // Number of readings to average

    for (int i = 0; i < numReadings; i++) {
        mag_x = read16(OUT_X_H_M, LSM303DLHC_MAG_ADDR);
        mag_y = read16(OUT_Y_H_M, LSM303DLHC_MAG_ADDR);
        mag_z = read16(OUT_Z_H_M, LSM303DLHC_MAG_ADDR);

        double mag_x_uT = mag_x * SCALE_FACTOR_MAG;
        double mag_y_uT = mag_y * SCALE_FACTOR_MAG;

        double heading = atan2(mag_y_uT, mag_x_uT) * 180.0 / 3.14159265358979323846;
        if (heading < 0) {
            heading += 360.0;
        }

        headingSum += heading;
        sleep_ms(100); // Wait a bit between readings
    }

    double heading = headingSum / numReadings;
    printf("%s Heading: %.2f degrees\n", direction, heading);
}

void magnometer_task(void *params) {
    stdio_init_all();
    
    // Initialize I2C at 100kHz on GP0 (SDA) and GP1 (SCL)
    i2c_init(i2c0, 100000);
    
    // Set GPIO 0 and GPIO 1 for I2C functionality
    gpio_set_function(0, GPIO_FUNC_I2C);  // GP0 = SDA
    gpio_set_function(1, GPIO_FUNC_I2C);  // GP1 = SCL
    gpio_pull_up(0);  // Pull-up on SDA
    gpio_pull_up(1);  // Pull-up on SCL

    // Now proceed with initializing the sensor and communicating with it...
    
    // Initialize magnetometer
    if (!i2c_write_blocking(i2c0, LSM303DLHC_MAG_ADDR, (const uint8_t[2]){CRA_REG_M, 0x14}, 2, true) ||
        !i2c_write_blocking(i2c0, LSM303DLHC_MAG_ADDR, (const uint8_t[2]){CRB_REG_M, 0x60}, 2, true) ||
        !i2c_write_blocking(i2c0, LSM303DLHC_MAG_ADDR, (const uint8_t[2]){MR_REG_M, 0x00}, 2, true)) {
        printf("Magnetometer I2C write error\n");
    }

    // Initialize accelerometer
    if (!i2c_write_blocking(i2c0, LSM303DLHC_ACC_ADDR, (const uint8_t[2]){CTRL_REG1_A, 0x57}, 2, true)) {
        printf("Accelerometer I2C write error\n");
    }

    wait_and_set_direction("NORTH", 15000);
    wait_and_set_direction("SOUTH", 15000);
    wait_and_set_direction("EAST", 15000);
    wait_and_set_direction("WEST", 15000);

    while (1) {
        int16_t mag_x = read16(OUT_X_H_M, LSM303DLHC_MAG_ADDR);
        int16_t mag_y = read16(OUT_Y_H_M, LSM303DLHC_MAG_ADDR);
        int16_t mag_z = read16(OUT_Z_H_M, LSM303DLHC_MAG_ADDR);

        int16_t acc_x = read16(OUT_X_L_A, LSM303DLHC_ACC_ADDR);
        int16_t acc_y = read16(OUT_Y_L_A, LSM303DLHC_ACC_ADDR);
        int16_t acc_z = read16(OUT_Z_L_A, LSM303DLHC_ACC_ADDR);

        // Apply scaling for magnetometer and accelerometer
        double mag_x_uT = mag_x * SCALE_FACTOR_MAG;
        double mag_y_uT = mag_y * SCALE_FACTOR_MAG;
        double mag_z_uT = mag_z * SCALE_FACTOR_MAG;

        double acc_x_g = acc_x * SCALE_FACTOR_ACC;
        double acc_y_g = acc_y * SCALE_FACTOR_ACC;
        double acc_z_g = acc_z * SCALE_FACTOR_ACC;

        // Print values
        printf("Magnetometer - X: %.2f uT, Y: %.2f uT, Z: %.2f uT\n", mag_x_uT, mag_y_uT, mag_z_uT);
        printf("Accelerometer - X: %.2f g, Y: %.2f g, Z: %.2f g\n", acc_x_g, acc_y_g, acc_z_g);

        sleep_ms(1000);
    }
}

int main() {
    stdio_init_all();  // Initialize I/O for printf
    magnometer_task(NULL);  // Start the magnetometer task
    return 0;  // Return 0 on successful execution
}