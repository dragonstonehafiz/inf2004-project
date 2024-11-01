#ifndef IR_SENSOR_H
#define IR_SENSOR_H

#include <stdint.h>
#include <stdbool.h>

// IR sensor configuration
#define IR_THRESHOLD 1.5f
#define IR_SENSOR_PIN 6 // Analog pin for IR sensor
#define PULSE_PIN 7     // Digital pin for pulse detection
#define WIDE_FACTOR 2.5f
#define PULSE_WIDTH_THRESHOLD 1000   // 0.1 second threshold for pulse width debounce
#define TIMEOUT_THRESHOLD_US 2000000 // 1-second timeout threshold in microseconds
#define MAX_BARCODE_LENGTH 29
#define BYTES_LENGTH 9
#define BARCODE_BITS_LENGTH MAX_BARCODE_LENGTH *BYTES_LENGTH
// Initializes the IR sensor module with callbacks for segment recording and timeout handling
void ir_sensor_init(void);

// Reads the analog value from the IR sensor and returns it as a voltage level
float ir_sensor_read(void);

void handle_edge_transition(unsigned int gpio, uint32_t events);

// Determines if a pulse width qualifies as "wide" based on a predefined wide factor
bool is_wide(int64_t width, int64_t narrow_width);

#endif // IR_SENSOR_H
