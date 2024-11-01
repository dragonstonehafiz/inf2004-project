// encoder.h

#ifndef _ENCODER_H
#define _ENCODER_H

#include <stdint.h> 

// Definitions for encoder GPIO pins and constants for calculations
#define LEFT_ENCODER_PIN 0        
#define RIGHT_ENCODER_PIN 2        
#define NOTCHES_PER_CYCLE 20       
#define CM_PER_NOTCH 1.0           

// Function declarations for encoder operations
void encoderCallback(unsigned int gpio, uint32_t events);  
double getLeftSpeed(void *params);
double getRightSpeed(void *params);
uint32_t getLeftNotchCount(void *params);
uint32_t getRightNotchCount(void *params);
void printEncoderData(void);       
void setupEncoderPins(void);    

#endif 
