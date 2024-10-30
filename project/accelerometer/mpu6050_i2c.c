#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/i2c.h"
#include <math.h>

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

#ifdef i2c_default  // Check if i2c_default is defined
// Function to initialize the LSM303DLHC accelerometer and magnetometer
static void lsm303dlhc_init() {
    uint8_t buf[2];

    // Initialize accelerometer: 100Hz, all axes enabled
    buf[0] = LSM303_REG_ACCEL_CTRL_REG1_A;
    buf[1] = 0x57; // 0b01010111: 100Hz, all axes enabled
    if (i2c_write_blocking(i2c_default, LSM303_ADDR_ACCEL, buf, 2, false) < 0) {
        printf("Failed to initialize accelerometer\n");
    }

    // Initialize magnetometer: Continuous conversion mode, 220 Hz
    buf[0] = LSM303_REG_MAG_CRA_REG_M;
    buf[1] = 0x1C; // 0b00011100: Continuous conversion, 220 Hz
    if (i2c_write_blocking(i2c_default, LSM303_ADDR_MAG, buf, 2, false) < 0) {
        printf("Failed to initialize magnetometer\n");
    }
}

// Function to scan I2C bus for connected devices
static void i2c_scan() {
    printf("I2C scan...\n");
    for (int addr = 0; addr < 127; addr++) {
        if (i2c_write_blocking(i2c_default, addr, NULL, 0, false) >= 0) {
            printf("Found I2C device at address 0x%02x\n", addr);
        }
    }
}

// Function to read raw accelerometer and magnetometer data
static void lsm303dlhc_read_raw(int16_t accel[3], int16_t mag[3]) {
    uint8_t buffer[6];

    // Read accelerometer data
    if (i2c_write_blocking(i2c_default, LSM303_ADDR_ACCEL, (uint8_t[]){LSM303_REG_ACCEL_OUT_X_L_A | 0x80}, 1, true) < 0) {
        printf("Failed to write to accelerometer\n");
        return;
    }
    if (i2c_read_blocking(i2c_default, LSM303_ADDR_ACCEL, buffer, 6, false) < 0) {
        printf("Failed to read accelerometer data\n");
        return;
    }
    for (int i = 0; i < 3; i++) {
        accel[i] = (int16_t)((buffer[2 * i + 1] << 8) | buffer[2 * i]); // Combine high and low byte
    }

    // Read magnetometer data
    if (i2c_write_blocking(i2c_default, LSM303_ADDR_MAG, (uint8_t[]){LSM303_REG_MAG_OUT_X_H_M}, 1, true) < 0) {
        printf("Failed to write to magnetometer\n");
        return;
    }
    if (i2c_read_blocking(i2c_default, LSM303_ADDR_MAG, buffer, 6, false) < 0) {
        printf("Failed to read magnetometer data\n");
        return;
    }
    for (int i = 0; i < 3; i++) {
        mag[i] = (int16_t)((buffer[2 * i] << 8) | buffer[2 * i + 1]); // Combine high and low byte
    }
}

// Function to calculate pitch, roll, and yaw
static void calculate_angles(int16_t accel[3], int16_t mag[3], float* pitch, float* roll, float* yaw) {
    // Convert accelerometer data to g's
    float ax = accel[0] * ACCEL_SCALE;
    float ay = accel[1] * ACCEL_SCALE;
    float az = accel[2] * ACCEL_SCALE;

    // Calculate pitch and roll
    *pitch = atan2(ay, sqrt(ax * ax + az * az));  // Pitch in radians
    *roll = atan2(ax, sqrt(ay * ay + az * az));   // Roll in radians

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

// Function to generate control commands based on sensor data
static void generate_remote_control_commands(float pitch, float roll) {
    // Print raw pitch and roll values
    printf("Debug: Raw Pitch = %.2f, Raw Roll = %.2f\n", pitch, roll);

    // Check for forward or backward movement based on pitch angle
    if (pitch > FORWARD_BACKWARD_THRESHOLD) {
        // Moving forward
        float speed_percentage = (pitch / 90.0) * MAX_SPEED;
        if (speed_percentage > MAX_SPEED) speed_percentage = MAX_SPEED;
        printf("Command: Move forward at %.1f%% speed\n", speed_percentage);
    } else if (pitch < -FORWARD_BACKWARD_THRESHOLD) {
        // Moving backward
        float speed_percentage = (-pitch / 90.0) * MAX_SPEED;
        if (speed_percentage > MAX_SPEED) speed_percentage = MAX_SPEED;
        printf("Command: Move backward at %.1f%% speed\n", speed_percentage);
    } else {
        // No forward or backward movement
        printf("Command: Stop moving forward/backward\n");
    }

    // Check for turning left or right based on roll angle
    if (roll > TURN_THRESHOLD) {
        // Turning right
        float turn_percentage = (roll / 90.0) * MAX_SPEED;
        if (turn_percentage > MAX_SPEED) turn_percentage = MAX_SPEED;
        printf("Command: Turn right at %.1f%% speed\n", turn_percentage);
    } else if (roll < -TURN_THRESHOLD) {
        // Turning left
        float turn_percentage = (-roll / 90.0) * MAX_SPEED;
        if (turn_percentage > MAX_SPEED) turn_percentage = MAX_SPEED;
        printf("Command: Turn left at %.1f%% speed\n", turn_percentage);
    } else {
        // No turning
        printf("Command: Stop turning\n");
    }
}

int main() {
    stdio_init_all();

#if !defined(i2c_default) || !defined(PICO_DEFAULT_I2C_SDA_PIN) || !defined(PICO_DEFAULT_I2C_SCL_PIN)
    #warning i2c/lsm303dlhc example requires a board with I2C pins
    puts("Default I2C pins were not defined");
    return 0;
#else
    printf("Hello, LSM303DLHC! Reading raw and adjusted data from registers...\n");

    // Initialize I2C at 400kHz
    i2c_init(i2c_default, 400 * 1000);
    gpio_set_function(PICO_DEFAULT_I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(PICO_DEFAULT_I2C_SDA_PIN);
    gpio_pull_up(PICO_DEFAULT_I2C_SCL_PIN);
    bi_decl(bi_2pins_with_func(PICO_DEFAULT_I2C_SDA_PIN, PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C));

    // Scan for I2C devices
    i2c_scan();

    // Initialize the LSM303DLHC sensor
    lsm303dlhc_init();

    int16_t acceleration[3], mag[3];
    float pitch, roll, yaw;

    // Main loop
    while (1) {
        // Read raw accelerometer and magnetometer data
        lsm303dlhc_read_raw(acceleration, mag);

        // Calculate the angles based on accelerometer and magnetometer data
        calculate_angles(acceleration, mag, &pitch, &roll, &yaw);

        // Print raw pitch and roll values
        printf("Raw Pitch = %.2f°, Raw Roll = %.2f°\n", pitch * 180.0 / M_PI, roll * 180.0 / M_PI);

        // Generate remote control commands based on raw pitch and roll
        generate_remote_control_commands(pitch * 180.0 / M_PI, roll * 180.0 / M_PI);

        sleep_ms(500);  // Shorter delay for more responsiveness
    }
#endif
}
#endif  // Ending the i2c_default block
