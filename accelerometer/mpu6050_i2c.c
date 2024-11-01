#include "mpu6050_i2c.h"  // Include the header file with declarations

#ifdef i2c_default  // Check if i2c_default is defined
float filtered_accel[3] = {0, 0, 0};
// Function to initialize the LSM303DLHC accelerometer
void lsm303dlhc_init() {
    uint8_t buf[2];

    // Initialize accelerometer: 100Hz, all axes enabled
    buf[0] = LSM303_REG_ACCEL_CTRL_REG1_A;
    buf[1] = 0x57; // 0b01010111: 100Hz, all axes enabled
    if (i2c_write_blocking(i2c_default, LSM303_ADDR_ACCEL, buf, 2, false) < 0) {
        // printf("Failed to initialize accelerometer\n");
    }
}

// Function to scan I2C bus for connected devices
void i2c_scan() {
    // printf("I2C scan...\n");
    for (int addr = 0; addr < 127; addr++) {
        if (i2c_write_blocking(i2c_default, addr, NULL, 0, false) >= 0) {
            // printf("Found I2C device at address 0x%02x\n", addr);
        }
    }
}

// Function to read and filter accelerometer data for stability
void lsm303dlhc_read_filtered_accel(int16_t accel[3]) {
    uint8_t buffer[6];

    // Read accelerometer data
    if (i2c_write_blocking(i2c_default, LSM303_ADDR_ACCEL, (uint8_t[]){LSM303_REG_ACCEL_OUT_X_L_A | 0x80}, 1, true) < 0) {
        // printf("Failed to write to accelerometer\n");
        return;
    }
    if (i2c_read_blocking(i2c_default, LSM303_ADDR_ACCEL, buffer, 6, false) < 0) {
        // printf("Failed to read accelerometer data\n");
        return;
    }

    // Apply filtering for stability
    for (int i = 0; i < 3; i++) {
        int16_t raw_data = (int16_t)((buffer[2 * i + 1] << 8) | buffer[2 * i]); // Combine high and low byte
        float new_data = raw_data * ACCEL_SCALE;
        filtered_accel[i] = ALPHA * new_data + (1 - ALPHA) * filtered_accel[i];  // Apply low-pass filter
        accel[i] = (int16_t)(filtered_accel[i] / ACCEL_SCALE);  // Store filtered data
    }
}

// Function to calculate pitch and roll in degrees from accelerometer data
void calculate_angles(int16_t accel[3], float* pitch, float* roll) {
    // Convert accelerometer data to g's
    float ax = accel[0] * ACCEL_SCALE;
    float ay = accel[1] * ACCEL_SCALE;
    float az = accel[2] * ACCEL_SCALE;

    // Calculate pitch and roll in radians
    *pitch = atan2(ay, sqrt(ax * ax + az * az));  // Pitch in radians
    *roll = atan2(ax, sqrt(ay * ay + az * az));   // Roll in radians

    // Convert pitch and roll to degrees
    *pitch *= (180.0 / M_PI);
    *roll *= (180.0 / M_PI);
}

// Function to generate forward/backward commands based on pitch
void generate_forward_backward_command(float pitch) {
    if (pitch > FORWARD_BACKWARD_THRESHOLD) {
        // Moving forward
        float speed_percentage = (pitch / 90.0) * MAX_SPEED;
        if (speed_percentage > MAX_SPEED) speed_percentage = MAX_SPEED;
        // printf("Command: Move forward at %.1f%% speed\n", speed_percentage);
    } else if (pitch < -FORWARD_BACKWARD_THRESHOLD) {
        // Moving backward
        float speed_percentage = (-pitch / 90.0) * MAX_SPEED;
        if (speed_percentage > MAX_SPEED) speed_percentage = MAX_SPEED;
        // printf("Command: Move backward at %.1f%% speed\n", speed_percentage);
    } else {
        // No forward or backward movement
        // printf("Command: Stop moving forward/backward\n");
    }
}

// Function to generate turn commands based on roll
void generate_turn_command(float roll) {
    if (roll > TURN_THRESHOLD) {
        // Turning right
        float turn_percentage = (roll / 90.0) * MAX_SPEED;
        if (turn_percentage > MAX_SPEED) turn_percentage = MAX_SPEED;
        // printf("Command: Turn right at %.1f%% speed\n", turn_percentage);
    } else if (roll < -TURN_THRESHOLD) {
        // Turning left
        float turn_percentage = (-roll / 90.0) * MAX_SPEED;
        if (turn_percentage > MAX_SPEED) turn_percentage = MAX_SPEED;
        // printf("Command: Turn left at %.1f%% speed\n", turn_percentage);
    } else {
        // No turning
        // printf("Command: Stop turning\n");
    }
}

int main() {
    stdio_init_all();

#if !defined(i2c_default) || !defined(PICO_DEFAULT_I2C_SDA_PIN) || !defined(PICO_DEFAULT_I2C_SCL_PIN)
    #warning i2c/lsm303dlhc example requires a board with I2C pins
    puts("Default I2C pins were not defined");
    return 0;
#else
    // printf("Hello, LSM303DLHC! Reading raw and adjusted data from registers...\n");

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

    int16_t acceleration[3];
    float pitch, roll;

    // Main loop
    while (1) {
        // Read filtered accelerometer data for stability
        lsm303dlhc_read_filtered_accel(acceleration);

        // Calculate the angles based on accelerometer data
        calculate_angles(acceleration, &pitch, &roll);

        // Print pitch and roll values in degrees
        // printf("Pitch = %.2f°, Roll = %.2f°\n", pitch, roll);

        // Generate commands based on pitch and roll
        generate_forward_backward_command(pitch);
        generate_turn_command(roll);

        sleep_ms(1000);  // Shorter delay for more responsiveness
    }
#endif
}
#endif  // Ending the i2c_default block
