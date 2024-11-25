#include "network.h"
#include "accelerometer.h"
#include "pins.h"

#define CONTROLLER_BUTTON_PIN 20 // Define the GPIO pin for the button
#define AUTO_BUTTON_PIN 21       // Btn to auto run the robot

int main()
{
    bool connected = false;
    bool is_running = false;       // Flag to track if we're sending data
    bool prev_button_state = true; // true because of pull-up (not pressed)
    bool is_auto = false;

    stdio_init_all();

    sleep_ms(3000); // Delay to allow serial to be opened
    // Initialize accelerometer
    init_accelerometer();
    init_server();
    printf("Accelerometer initialized\n");

    // Initialize button GPIO with pull-up
    gpio_init(CONTROLLER_BUTTON_PIN);
    gpio_set_dir(CONTROLLER_BUTTON_PIN, GPIO_IN);
    gpio_pull_up(CONTROLLER_BUTTON_PIN);

    gpio_init(AUTO_BUTTON_PIN);
    gpio_set_dir(AUTO_BUTTON_PIN, GPIO_IN);
    gpio_pull_up(AUTO_BUTTON_PIN);

    // Connect to wifi
    connected = connect_to_wifi();

    if (connected)
    {
        if (init_udp_server_sender(IP_CAR) == NULL)
        {
            printf("UDP initialization failed\n");
            connected = false;
        }
        else
        {
            printf("UDP initialized successfully\n");
        }
    }
    else
    {
        printf("WiFi connection failed\n");
    }

    while (connected)
    {

        // Check button state
        bool current_button_state = gpio_get(CONTROLLER_BUTTON_PIN);
        bool auto_button_state = gpio_get(AUTO_BUTTON_PIN);

        if (!auto_button_state && !is_auto)
        {
            // Set Direction to T which means to toggle to auto drive
            data_to_send_t movement_data = {'T', 0.0f, 'N', 0.0f};
            send_udp_data(serialize_movement_data(&movement_data), PORT_CAR, IP_CAR);
            is_auto = true;
        }

        // Detect button press (transition from high to low due to pull-up)
        if (prev_button_state && !current_button_state && !is_auto)
        {
            is_running = !is_running; // Toggle the state

            if (is_running)
            {
                printf("Started sending data\n");
            }
            else
            {
                printf("Stopped sending data\n");
            }

            sleep_ms(50); // Simple debounce
        }
        prev_button_state = current_button_state;

        // Only call get_data_to_send if is_running is true
        if (is_running)
        {
            char *toSend = get_data_to_send(); // This function will handle both getting and sending data
            if (toSend != NULL)
                send_udp_data(toSend, PORT_CAR, IP_CAR);
        }

        sleep_ms(100);
    }

    cyw43_arch_deinit();
    return 0;
}