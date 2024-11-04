#include <string.h>
#include <stdio.h>
#include "code39_decoder.h"
#include "ir_sensor.h"
#include "utils.h"

const char *code39_chars = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ*";
const char *code39_encoding[] = {
    "000110100", "100100001", "001100001", "101100000", "000110001",
    "100110000", "001110000", "000100101", "100100100", "001100100",
    "100001001", "001001001", "101001000", "000011001", "100011000",
    "001011000", "000001101", "100001100", "001001100", "000011100",
    "100000011", "001000011", "101000010", "000010011", "100010010",
    "001010010", "000000111", "100000110", "001000110", "000010110",
    "110000001", "011000001", "111000000", "010010001", "110010000",
    "011010000", "010010100"};

char decode_code39_character(char chunk[], int length)
{
    size_t encoding_count = sizeof(code39_encoding) / sizeof(code39_encoding[0]);
    for (size_t i = 0; i < encoding_count; i++)
    {
        if (strncmp(chunk, code39_encoding[i], length) == 0)
        {
            // printf("Decoded character: %c\n", code39_chars[i]);
            return code39_chars[i];
        }
    }
    return '?';
}

int decode_code39_sequence(int64_t widths[], char *output)
{
    size_t output_index = 0;

    char pattern[MAX_BARCODE_LENGTH + 1] = {0};

    classify_mean_std(widths, MAX_BARCODE_LENGTH, pattern, 0.3f);

    pattern[MAX_BARCODE_LENGTH] = '\0';

    // printf("Pattern: %s\n", pattern);

    char start_char = decode_code39_character(&pattern[0], BYTES_LENGTH);

    if (start_char != '*')
    {
        // printf("Invalid start character. %c\n", start_char);
        return 0;
    }

    char decoded_char = decode_code39_character(&pattern[BYTES_LENGTH + 1], BYTES_LENGTH);
    output[output_index++] = decoded_char;

    output[output_index] = '\0'; // Null-terminate the output string
    // printf("Output: %s\n", output);
    return 1; // Return true if decoding succeeds
}

void reverse_widths(int64_t *widths, int length)
{
    for (int i = 0; i < length / 2; i++)
    {
        int64_t temp = widths[i];
        widths[i] = widths[length - 1 - i];
        widths[length - 1 - i] = temp;
    }
}

int decode_with_direction_check(int64_t *widths)
{
    // print_bar_space_widths(widths, MAX_BARCODE_LENGTH);

    char decoded_normal[2];
    char decoded_reversed[2];

    // Try decoding in normal order
    if (decode_code39_sequence(widths, decoded_normal))
    {
        printf("Decoded barcode (Normal): %s\n", decoded_normal);
        return 1;
    }

    // Reverse widths and try decoding in reverse order
    reverse_widths(widths, MAX_BARCODE_LENGTH);

    if (decode_code39_sequence(widths, decoded_reversed))
    {
        printf("Decoded barcode (Reversed): %s\n", decoded_reversed);
        return 1;
    }

    return 0;
}