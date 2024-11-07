#ifndef IR_SENSOR_H
#define IR_SENSOR_H

#include <stdint.h>
#include <stdbool.h>

// IR sensor configuration
#define IR_THRESHOLD 1.5f
#define IR_SENSOR_PIN 27    // Analog pin for IR sensor
#define PULSE_PIN_BARCODE 6 // Digital pin for pulse detection
#define PIN_LINE_TRACING 17
#define WIDE_FACTOR 2.5f
#define PULSE_WIDTH_THRESHOLD 1000   // 0.001 second threshold for pulse width debounce
#define TIMEOUT_THRESHOLD_US 750000 // 0.75-second timeout threshold in microseconds
#define MAX_BARCODE_LENGTH 29
#define BYTES_LENGTH 9
#define BARCODE_BITS_LENGTH MAX_BARCODE_LENGTH *BYTES_LENGTH

#define BLACK_DETECTED 1
#define WHITE_DETECTED 0

// Initializes the IR sensor module with callbacks for segment recording and timeout handling
void ir_sensor_init(void);

// Reads the analog value from the IR sensor and returns it as a voltage level
float ir_sensor_read(void);

void create_line_tracing_transaction(unsigned int gpio, uint32_t events);
void handle_line_tracing(void (*callback)(uint8_t movement_state));

void create_barcode_transaction(unsigned int gpio, uint32_t events);
void handle_barcode(void);

void reset_ir_sensor_status(void);
#endif // IR_SENSOR_H
