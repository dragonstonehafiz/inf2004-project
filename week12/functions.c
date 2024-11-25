#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include "functions.h"

#define LED_PIN 16
#define LED_PIN2 17

#define BTN_DOT 22
#define BTN_DASH 21
#define BTN_OUTPUT 20

#define PWM_WRAP 65535
#define PWM_FREQ 300
#define CLOCK_FREQ 125000000

#define LOW_DUTY_CYCLE 0.1f

uint led[] = {13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2};
char state[] = {
    ' ', ' ', 
    ' ', ' ', 
    ' ', ' ', 
    ' ', ' ', 
    ' ', ' ', 
    ' ', ' ', 
    ' ', ' '
    };
uint8_t currIndex = 0;
uint8_t maxIndex = 11;
uint8_t renderIndex = 0;
bool playback = false;

bool buttonRegister = true;

void initButton(uint gpio)
{
    gpio_init(gpio);
    gpio_set_dir(gpio, GPIO_IN);
    gpio_pull_up(gpio);
}
void initLED(uint gpio)
{
    gpio_init(gpio);
    gpio_set_dir(gpio, GPIO_OUT);

    setupPWM(gpio, 0.f);
}
void init()
{
    // Buttons
    initButton(BTN_DASH);
    initButton(BTN_DOT);
    initButton(BTN_OUTPUT);
    gpio_set_irq_enabled_with_callback(BTN_DASH, GPIO_IRQ_EDGE_FALL, true, irq_func);
    gpio_set_irq_enabled_with_callback(BTN_DOT, GPIO_IRQ_EDGE_FALL, true, irq_func);
    gpio_set_irq_enabled_with_callback(BTN_OUTPUT, GPIO_IRQ_EDGE_FALL, true, irq_func);

    // LEDs
    for (int i = 0; i <= maxIndex; ++i)
        initLED(led[i]);
}

void setupPWM(uint pwm_pin, float duty_cycle) 
{
    // Calculate the PWM frequency and set the PWM wrap value
    uint32_t divider = CLOCK_FREQ / (PWM_FREQ * PWM_WRAP);  // Compute divider for given frequency

    // Set the GPIO function to PWM
    gpio_set_function(pwm_pin, GPIO_FUNC_PWM);

    // Find out which PWM slice is connected to the specified GPIO
    uint slice_num = pwm_gpio_to_slice_num(pwm_pin);
    pwm_set_clkdiv(slice_num, divider);
    pwm_set_wrap(slice_num, PWM_WRAP);
    pwm_set_gpio_level(pwm_pin, (uint16_t)(duty_cycle * (PWM_WRAP + 1)));
    setPWMDutyCycle(pwm_pin, duty_cycle);

    pwm_set_enabled(slice_num, true);
}
void setPWMDutyCycle(uint pwm_pin, float duty_cycle) 
{
    pwm_set_gpio_level(pwm_pin, (uint16_t)((duty_cycle) * (PWM_WRAP - 1)));
}

void irq_func(uint gpio, uint32_t events)
{
    if (!buttonRegister)
        return;

    if (gpio == BTN_DASH)
    {
        if (!addChar('-'))
            printf("Max Capacity Reached\n");
        else
            printf("Dash Added: [%d]\n", currIndex - 1);
    }
    else if (gpio == BTN_DOT)
    {
        if (!addChar('.'))
            printf("Max Capacity Reached\n");
        else
            printf("Dot Added: [%d]\n", currIndex - 1);
    }
    else if (gpio == BTN_OUTPUT)
    {
        startPlayback();
    }

    // Disable Button
    buttonRegister = false;
    // Timer to reenable button
    add_alarm_in_ms(250, reeanbleButtons, NULL, false);
}

bool addChar(char toAdd)
{
    if (currIndex > maxIndex)
        return false;
    else
    {
        state[currIndex] = toAdd;
        currIndex += 1;
        return true;
    }
}
void setLED(uint gpio, char toPrint)
{   
    if (toPrint == '-')
        setPWMDutyCycle(gpio, LOW_DUTY_CYCLE);
    else if (toPrint == '.')
        setPWMDutyCycle(gpio, 1.f);
}

void startPlayback()
{
    if (!playback)
    {
        playback = true;
        renderIndex = 0;
        printf(">> Code Playback Start\n");
    }
}
void mainloop()
{
    if (playback)
    {
        uint gpio = led[renderIndex];
        char gpio_state = state[renderIndex];
        if (gpio_state == '-')
        {
            setLED(gpio, gpio_state);
            printf("Dash at [%d]\n", renderIndex);
        }
        else if (gpio_state == '.')
        {
            setLED(gpio, gpio_state);
            printf("Dot at [%d]\n", renderIndex);
        }
        else
        {
            playback = false;
            printf(">> Code Playback End\n");
            for (int i = 0; i <= maxIndex; ++i)
            {
                uint gpio = led[i];
                setPWMDutyCycle(gpio, 0.f);
                // state[i] = ' ';
            }
            currIndex = 0;
        }
        renderIndex += 1;

        sleep_ms(1000);
    }
}

int64_t reeanbleButtons(alarm_id_t id, void *user_data) 
{
    // Reset the interrupt.
    buttonRegister = true;
    return 0;
}
