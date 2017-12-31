// free bitmap for OS file systems assignment

#include "bitmap.h"
#include "src/geom.h"

#include <strings.h>    // for `ffs`
#include <stdlib.h>

/* constants */
// how far to loop in array
/* globals */

/* macros */
#define FREE_BIT(_data, _which_bit) \
    _data = _data | (1 << _which_bit)

#define USE_BIT(_data, _which_bit) \
    _data = _data & ~(1 << _which_bit)

void force_set_index(uint32_t index, uint8_t* free_bit_map) {
	uint32_t i = index / 8; // Find byte
	uint8_t  bit = index % 8; // Find bit
	USE_BIT(free_bit_map[i],bit); // Use macro to set bit
}

uint32_t get_index(uint8_t* free_bit_map) {
    uint32_t i = 0;

    // find the first section with a free bit
    for (i = 0; i < LEN_BITMAP && free_bit_map[i] == 0; i++);
    if (i == LEN_BITMAP)
	    return -1;

    // now, find the first free bit
    /*
        The ffs() function returns the position of the first (least
       significant) bit set in the word i.  The least significant bit is
       position 1 and the most significant position is, for example, 32 or
       64.  
    */
    // Since ffs has the lsb as 1, not 0. So we need to subtract
    uint8_t bit = ffs(free_bit_map[i]) - 1;

    // set the bit to used
    USE_BIT(free_bit_map[i], bit);

    //return which block we used
    return i*8 + bit;
}

void rm_index(uint32_t index,uint8_t* free_bit_map) {

    // get index in array of which bit to free
    uint32_t i = index / 8;

    // get which bit to free
    uint8_t bit = index % 8;

    // free bit
    FREE_BIT(free_bit_map[i], bit);
}

