// code39_decoder.h
#ifndef CODE39_DECODER_H
#define CODE39_DECODER_H

#include <stdint.h>
#include <stddef.h>

// Decodes a single Code39 character based on segment widths and the narrow width threshold
char decode_code39_character(int64_t widths[], int64_t narrow_width);

// Decodes a sequence of Code39 characters, populating the output string and returning success or failure
int decode_code39_sequence(int64_t widths[], int64_t narrow_width, char *output);

int decode_with_direction_check(int64_t *widths, int64_t narrow_width);

#endif // CODE39_DECODER_H
