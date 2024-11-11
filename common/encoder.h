#ifndef _ENCODER_H
#define _ENCODER_H

#include <stdio.h>
#include "pico/stdlib.h"

// Making these extern so you can get encoder distance just by including the encoder.h file
volatile extern double leftTotalDistance;
volatile extern double rightTotalDistance;
volatile extern double leftTotalDistance;
volatile extern double rightTotalDistance;

static inline void printEncoderData(void);
bool encoderTimerCallback(struct repeating_timer *t);
void encoderCallback(uint gpio, uint32_t events);
void checkIfStopped();
void setupEncoderPins();
void resetEncoder();

#endif
