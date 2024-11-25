#include "network.h"
#include "accelerometer.h"
#include "pins.h"

#define CONTROLLER_BUTTON_PIN 20 // Define the GPIO pin for the button
#define AUTO_BUTTON_PIN 21       // Btn to auto run the robot

bool is_auto = false;
bool is_sending = false;

void irq_handler(uint gpio, uint32_t events);

int main()
{

    stdio_init_all();
    // Initialize accelerometer
    init_accelerometer();

    gpio_set_irq_enabled_with_callback(CONTROLLER_BUTTON_PIN, GPIO_IRQ_EDGE_FALL, true, &irq_handler);
    gpio_set_irq_enabled_with_callback(AUTO_BUTTON_PIN, GPIO_IRQ_EDGE_FALL, true, &irq_handler);

    // Initialize button GPIO with pull-up
    gpio_init(CONTROLLER_BUTTON_PIN);
    gpio_set_dir(CONTROLLER_BUTTON_PIN, GPIO_IN);
    gpio_pull_up(CONTROLLER_BUTTON_PIN);

    gpio_init(AUTO_BUTTON_PIN);
    gpio_set_dir(AUTO_BUTTON_PIN, GPIO_IN);
    gpio_pull_up(AUTO_BUTTON_PIN);

    bool connected = false;
    // Connect to wifi
    init_server();
    connected = connect_to_wifi();

    if (connected)
    {
        if (init_udp_server_sender(IP_CAR) == NULL)
        {
            printf("UDP initialization failed\n");
            connected = false;
        }
        else
            printf("UDP initialized successfully\n");
    }
    else
        printf("WiFi connection failed\n");

    while (connected)
    {
        // Check button state
        bool current_button_state = gpio_get(CONTROLLER_BUTTON_PIN);
        bool auto_button_state = gpio_get(AUTO_BUTTON_PIN);

        // Only send messages if not in auto
        if (is_sending && !is_auto)
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

void irq_handler(uint gpio, uint32_t events)
{
    if (gpio == CONTROLLER_BUTTON_PIN)
        is_sending = !is_sending;
    if (gpio == AUTO_BUTTON_PIN && !is_auto)
    {
        data_to_send_t movement_data = {'T', 0.0f, 'N', 0.0f};
        send_udp_data(serialize_movement_data(&movement_data), PORT_CAR, IP_CAR);
        is_auto = true;
    }
}