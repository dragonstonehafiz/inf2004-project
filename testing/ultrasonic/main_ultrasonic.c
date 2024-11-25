#include "pico/stdlib.h"
#include "stdio.h"
#include "ultrasonic.h"
#include "pins.h"

// Interval in milliseconds for checking the distance
#define CHECK_INTERVAL_MS 200
// Distance threshold for the buzzer activation in centimeters
#define DISTANCE_THRESHOLD_CM 10.0

int main()
{
    stdio_init_all();

    setupUltrasonicPins();
    setupBuzzerPin();

    uint64_t last_distance_check_time = time_us_64();
    while (1)
    {
        uint64_t current_time = time_us_64();

        // Check the distance at the specified interval
        if (current_time - last_distance_check_time > CHECK_INTERVAL_MS * 1000)
        {
            triggerPulse(); // Initiate the pulse to start the measurement

            sleep_ms(30); // Small delay to ensure echo is received if object is close
            float distanceCm = getCm();

            if (distanceCm == -1.0)
            {
                printf("No echo received (timeout).\n");
            }
            else
            {
                printf("Distance: %.2f cm\n", distanceCm);

                if (distanceCm < DISTANCE_THRESHOLD_CM)
                {
                    printf("Object too close! Buzzing....\n");
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