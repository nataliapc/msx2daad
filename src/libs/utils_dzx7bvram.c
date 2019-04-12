/*
	Original source:
		https://github.com/antoniovillena/zx7b

    ******* NOT TESTED *******
*/
#include <stdlib.h>
#include <stdint.h>
#include "vdp.h"


unsigned char *input_data;
unsigned char *output_data;
size_t input_index;
size_t output_index;
size_t partial_counter;
int bit_mask;
int bit_value;


int read_byte()
{
    return input_data[input_index++];
}

int read_bit() {
    bit_mask >>= 1;
    if (bit_mask == 0) {
        bit_mask = 128;
        bit_value = read_byte();
    }
    return bit_value & bit_mask ? 1 : 0;
}

int read_elias_gamma()
{
    int value;

    value = 1;
    while (!read_bit()) {
        value = value << 1 | read_bit();
    }
    if( (value&255)==255 )
      value= -1;
    return value;
}

int read_offset()
{
    int value;
    int i;

    value = read_byte();
    if (value < 128) {
        return value;
    } else {
        i = read_bit();
        i = i << 1 | read_bit();
        i = i << 1 | read_bit();
        i = i << 1 | read_bit();
        return (value & 127 | i << 7) + 128;
    }
}

void write_byte(int value) __naked
{
    value;
    __asm
            pop af
            pop bc
            push bc
            push af
            ld a,c
            out (0x98),a
    __endasm;
    //output_data[output_index++] = value;
}

void write_bytes(int offset, int length)
{
    while (length-- > 0) {
        write_byte(output_data[output_index-offset]);
    }
}

void dzx7bvram(char *input, char *output, uint32_t vram)
{
    int length;

    input_data = input;
    output_data = output;

    input_index = 0;
    partial_counter = 0;
    output_index = 0;
    bit_mask = 0;

    setVDP_Write(vram);

    write_byte(read_byte());
    while (1) {
        if (!read_bit()) {
            write_byte(read_byte());
        } else {
            length = read_elias_gamma()+1;
            if (length == 0) {
                return;
            }
            write_bytes(read_offset()+1, length);
        }
    }
}
