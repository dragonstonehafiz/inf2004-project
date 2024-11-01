#include "pico/stdlib.h"
#include <stdio.h>
#include "hardware/gpio.h"
#include "hardware/timer.h"
#include "include/hardware/ultrasonic.h"  

void setupUltrasonicPins(unsigned int trigPin, unsigned int echoPin)
{
    gpio_init(trigPin);
    gpio_init(echoPin);
    gpio_set_dir(trigPin, GPIO_OUT);  
    gpio_set_dir(echoPin, GPIO_IN);   
}

void setupBuzzerPin()
{
    gpio_init(BUZZER_PIN);
    gpio_set_dir(BUZZER_PIN, GPIO_OUT);  
}

uint64_t getPulse(unsigned int trigPin, unsigned int echoPin)
{
    gpio_put(trigPin, 0);  
    sleep_us(2);
    gpio_put(trigPin, 1);  
    sleep_us(10);          
    gpio_put(trigPin, 0);  

    uint64_t startWait = time_us_64();
    while (gpio_get(echoPin) == 0) {
        if (time_us_64() - startWait > 30000) {
            return 0;  
        }
    }

    uint64_t pulseStart = time_us_64();
    while (gpio_get(echoPin) == 1) {
        if (time_us_64() - pulseStart > 30000) {
            return 0;  
        }
    }
    uint64_t pulseEnd = time_us_64();

    return pulseEnd - pulseStart;
}

float getCm(unsigned int trigPin, unsigned int echoPin)
{
    uint64_t pulseLength = getPulse(trigPin, echoPin);
    if (pulseLength == 0) return 0.0;  
    return (float)pulseLength / 29.0 / 2.0; 
}

int main() {
    stdio_init_all();  

    setupUltrasonicPins(TRIG_PIN, ECHO_PIN);
    setupBuzzerPin();   

    uint64_t last_distance_check_time = time_us_64();  
    while (1) {
        uint64_t current_time = time_us_64();

        if (current_time - last_distance_check_time > CHECK_INTERVAL_MS * 1000) {
            float distanceCm = getCm(TRIG_PIN, ECHO_PIN);

            if (distanceCm == 0.0) {
                printf("No echo received (timeout).\n");
            } else {
                printf("Distance: %.2f cm\n", distanceCm);

                if (distanceCm < DISTANCE_THRESHOLD_CM) {
                    printf("Object too close! Buzzing...\n");
                    gpio_put(BUZZER_PIN, 1);  
                    sleep_ms(200);            
                    gpio_put(BUZZER_PIN, 0);  
                }
            }

            last_distance_check_time = current_time;
        }
    }

    return 0;
}
