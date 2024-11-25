#include "accelerometer.h"
#include <string.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/i2c.h"
#include <math.h>

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
#define ACCEL_SCALE 0.001        // Accelerometer scale for ±2g range (1 mg/LSB)
#define MAG_SCALE (1.0 / 1100.0) // Magnetometer scale for ±1.3 gauss range (1100 LSB/gauss)

// Thresholds for forward movement
#define THRESHOLD_FORWARD_DEG 15.0
#define THRESHOLD_FORWARD_RAD (THRESHOLD_FORWARD_DEG / 180.0 * M_PI) // Threshold for movement
#define MAX_ANGLE_FORWARD_RAD (90.0 / 180.0 * M_PI)
// Threshold for turning
#define THRESHOLD_TURN_DEG 10.0
#define THRESHOLD_TURN_RAD (THRESHOLD_TURN_DEG / 180.0 * M_PI)
#define MAX_ANGLE_TURN_RAD (60.0 / 180.0 * M_PI)

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
void calculate_angles(float *pitch, float *roll, float *yaw)
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
    *pitch = atan2(ay, sqrt(ax * ax + azaz)); // Pitch in radians
    *roll = atan2(ax, sqrt(ay * ay + azaz));  // Roll in radians

    // Convert magnetometer data to gauss
    float mx = mag[0] * MAG_SCALE;
    float my = mag[1] * MAG_SCALE;
    float mz = mag[2] * MAG_SCALE;

    // Adjust magnetometer readings by pitch and roll
    float mx2 = mx * cos(*pitch) + mz * sin(*pitch);
    float my2 = mx * sin(*roll) * sin(*pitch) + my * cos(*roll) - mz * sin(*roll) * cos(*pitch);

    // Calculate yaw (compass heading)
    *yaw = atan2(my2, mx2); // Yaw in radians
}

float convert_to_discrete_percentage_forward(float pitch)
{
    float input_percentage = pitch / MAX_ANGLE_FORWARD_RAD;
    if (input_percentage > 1.f)
        input_percentage = 1.f;
    int levels = ceil(MAX_ANGLE_FORWARD_RAD / THRESHOLD_FORWARD_RAD);
    float step = 1.0f / levels;
    float currLevel = (input_percentage * levels);
    // printf("levels: %d, inputPercent: %.2f, currLevel: %d\n", levels, input_percentage, (int)round(currLevel));
    if (input_percentage < step)
        return 0.0f;
    else if (input_percentage > 1.0f - step)
        return 1.0f;
    else
        return (int)(currLevel)*step;
}

float convert_to_discrete_percentage_turning(float roll)
{
    float input_percentage = roll / MAX_ANGLE_TURN_RAD;
    if (input_percentage > 1.f)
        input_percentage = 1.f;
    int levels = ceil(MAX_ANGLE_TURN_RAD / THRESHOLD_TURN_RAD);
    float step = 1.0f / levels;
    float currLevel = (input_percentage * levels);
    // printf("levels: %d, inputPercent: %.2f, currLevel: %d\n", levels, input_percentage, (int)round(currLevel));
    if (input_percentage < step)
        return 0.0f;
    else if (input_percentage > 1.0f - step)
        return 1.0f;
    else
        return (int)(currLevel)*step;
}

// Get forward and turn command at once
data_to_send_t get_command(float pitch, float roll, data_to_send_t movement)
{
    if (pitch > THRESHOLD_FORWARD_RAD * 1.5)
    {
        // Moving forward
        movement.forward_direction = 'F';
        movement.forward_percentage = convert_to_discrete_percentage_forward(pitch);
    }
    else if (pitch < -THRESHOLD_FORWARD_RAD * 1.5)
    {
        // Moving backward
        movement.forward_direction = 'B';
        movement.forward_percentage = convert_to_discrete_percentage_forward(-pitch);
    }
    else
    {
        movement.forward_direction = 'N';
        movement.forward_percentage = 0.0f; // Changed from speed_percentage
    }

    // Turn commands remain unchanged
    if (roll > THRESHOLD_TURN_RAD)
    {
        movement.turn_direction = 'L';
        movement.turn_percentage = convert_to_discrete_percentage_turning(roll);
    }
    else if (roll < -THRESHOLD_TURN_RAD)
    {
        movement.turn_direction = 'R';
        movement.turn_percentage = convert_to_discrete_percentage_turning(-roll);
    }
    else
    {
        movement.turn_direction = 'N';
        movement.turn_percentage = 0.0f;
    }

    return movement;
}

// Function to serialize movement data
char *serialize_movement_data(const data_to_send_t *data)
{
    static char buffer[MAX_DATA_STRING];
    snprintf(buffer, sizeof(buffer), "PICO|%c:%.1f,%c:%.1f\n",
             data->forward_direction,
             data->forward_percentage, // Changed from speed_percentage
             data->turn_direction,
             data->turn_percentage);
    return buffer;
}

char *get_data_to_send(void)
{
    static float prev_forward_percentage = 0.0f; // Changed from prev_forward_speed
    static float prev_turn_speed = 0.0f;
    static char prev_forward_dir = 'N';
    static char prev_turn_dir = 'N';

    // Initialize movement_data_t
    data_to_send_t movement_data = {'N', 0.0f, 'N', 0.0f};

    float pitch, roll, yaw;
    char *data_to_send;

    // Read accelerometer data
    calculate_angles(&pitch, &roll, &yaw);

    // Process movement commands
    movement_data = get_command(pitch, roll, movement_data);

    // Check if current data is different from previous data
    if (movement_data.forward_direction != prev_forward_dir ||
        movement_data.forward_percentage != prev_forward_percentage || // Changed from prev_forward_speed
        movement_data.turn_direction != prev_turn_dir ||
        movement_data.turn_percentage != prev_turn_speed)
    {

        // Data has changed, send it
        data_to_send = serialize_movement_data(&movement_data);

        // Update previous values
        prev_forward_dir = movement_data.forward_direction;
        prev_forward_percentage = movement_data.forward_percentage; // Changed from prev_forward_speed
        prev_turn_dir = movement_data.turn_direction;
        prev_turn_speed = movement_data.turn_percentage;

        return data_to_send;
    }
    return NULL;
}
