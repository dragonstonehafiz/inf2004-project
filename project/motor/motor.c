#include "motor.h"

void init_motor()
{    
    init_left_motor();
    init_right_motor();
}
void init_left_motor()
{
    setup_pwm(WHEEL_LEFT_PWN_PIN, 0.f);
    gpio_init(WHEEL_LEFT_OUT_PIN_1);
    gpio_init(WHEEL_LEFT_OUT_PIN_2);
    gpio_set_dir(WHEEL_LEFT_OUT_PIN_1, GPIO_OUT);
    gpio_set_dir(WHEEL_LEFT_OUT_PIN_2, GPIO_OUT);
    turn_wheel_clockwise(WHEEL_LEFT_PWN_PIN);
}
void init_right_motor()
{
    setup_pwm(WHEEL_RIGHT_PWN_PIN, 0.f);
    gpio_init(WHEEL_RIGHT_OUT_PIN_1);
    gpio_init(WHEEL_RIGHT_OUT_PIN_2);
    gpio_set_dir(WHEEL_RIGHT_OUT_PIN_1, GPIO_OUT);
    gpio_set_dir(WHEEL_RIGHT_OUT_PIN_2, GPIO_OUT);
    turn_wheel_anticlockwise(WHEEL_RIGHT_PWN_PIN);
}

void setup_pwm(uint pwm_pin, float duty_cycle) 
{
    // Calculate the PWM frequency and set the PWM wrap value
    float clock_freq = 125000000.0f;  // Default Pico clock frequency in Hz
    uint16_t freq = 25;
    uint32_t divider = clock_freq / (freq * PWM_WRAP);  // Compute divider for given frequency

    // Set the GPIO function to PWM
    gpio_set_function(pwm_pin, GPIO_FUNC_PWM);

    // Find out which PWM slice is connected to the specified GPIO
    uint slice_num = pwm_gpio_to_slice_num(pwm_pin);
    pwm_set_clkdiv(slice_num, divider);
    pwm_set_wrap(slice_num, PWM_WRAP);
    pwm_set_gpio_level(pwm_pin, (uint16_t)(duty_cycle * (PWM_WRAP + 1)));
    set_pwm_duty_cycle(pwm_pin, &duty_cycle);

    pwm_set_enabled(slice_num, true);
}
void set_pwm_duty_cycle(uint pwm_pin, float *duty_cycle) 
{
    pwm_set_gpio_level(pwm_pin, (uint16_t)((*duty_cycle) * (PWM_WRAP - 1)));
}

void turn_wheel_clockwise(uint pwm_pin)
{
    if (pwm_pin == WHEEL_LEFT_PWN_PIN)
    {
        printf("left forward\n");
        gpio_put(WHEEL_LEFT_OUT_PIN_1, true);
        gpio_put(WHEEL_LEFT_OUT_PIN_2, false);
    } 
    else if (pwm_pin == WHEEL_RIGHT_PWN_PIN)
    {
        printf("right forward\n");
        gpio_put(WHEEL_RIGHT_OUT_PIN_1, true);
        gpio_put(WHEEL_RIGHT_OUT_PIN_2, false);
    }
}
void turn_wheel_anticlockwise(uint pwm_pin)
{
    if (pwm_pin == WHEEL_LEFT_PWN_PIN)
    {
        gpio_put(WHEEL_LEFT_OUT_PIN_1, false);
        gpio_put(WHEEL_LEFT_OUT_PIN_2, true);
    } 
    else if (pwm_pin == WHEEL_RIGHT_PWN_PIN)
    {
        gpio_put(WHEEL_RIGHT_OUT_PIN_1, false);
        gpio_put(WHEEL_RIGHT_OUT_PIN_2, true);
    }
}
void set_wheel_duty_cycle(uint pwm_pin, float* dutyCycle)
{
    if (pwm_pin == WHEEL_LEFT_PWN_PIN || pwm_pin == WHEEL_RIGHT_PWN_PIN)
        set_pwm_duty_cycle(pwm_pin, dutyCycle);
}

void compute_wheel_duty_cycle(float *target_speed, float *current_speed, float *duty_cycle, float *integral, float *prev_error)
{
    float error = *target_speed - *current_speed;
    *integral += error;
    float derivative = error - *prev_error;

    // float Kp = 0.1, Ki = 0.01, Kd = 0.005;
    *duty_cycle += 0.1 * error + 0.01 * (*integral) + 0.005 * derivative;

    // Clamp the duty cycle to the range [0, 1]
    if (*duty_cycle > 1.0) *duty_cycle = 1.0;
    else if (*duty_cycle < 0) *duty_cycle = 0;

    *prev_error = error;
}