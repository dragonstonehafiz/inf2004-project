// code39_decoder.h
#ifndef CODE39_DECODER_H
#define CODE39_DECODER_H

#include <stdint.h>
#include <stddef.h>

// Decodes a single Code39 character based on segment widths and the narrow width threshold
char decode_code39_character(char chunk[], int length);

// Decodes a sequence of Code39 characters, populating the output string and returning success or failure
int decode_code39_sequence(int64_t widths[], char *output);

int decode_with_direction_check(int64_t *widths);

#endif // CODE39_DECODER_H
