#ifndef UTILS_H
#define UTILS_H
#include <stdint.h>

void print_bar_space_widths(int64_t *widths, int count);

void classify_mean(int64_t arr[], int size, char result[]);

void classify_mean_std(int64_t arr[], int size, char result[], double factor);

void classify_median(int64_t arr[], int size, char result[]);

#endif // UTILS_H