#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

#include "../disk_emu.h"
#include "../bitmap.h"
#include "geom.h"
#include "blkio.h"

// Reads a contiguous data structure from a series of blocks
int struct_read(int base, int size, void** dest) {
	// 1. Calculate the number of blocks we must read to find this structure
	assert(base < N_BLOCKS);
	int blocks = (size + LEN_BLOCK - 1) / LEN_BLOCK; // Equivalent of ceil(size/LEN_BLOCK)


	// 2. Allocate our internal buffer to hold what we read
	void* buf = malloc(blocks * LEN_BLOCK);
	assert(buf != NULL);

	// 3. Retrieve the blocks from the disk
	int blocks_read = read_blocks(base,blocks,buf);
	assert(blocks_read == blocks); // In case read_blocks is borked

	// 4. Determine if we need to allocate the dest
	if (*dest == NULL)
		*dest = malloc(size);

	// 5. Copy in the data structure
	memcpy(*dest,buf,size);

	// 6. Release our internal buffer
	free(buf);

	return 0;
}

// Reads a chunk from a block, returning the number of bytes left to find elsewhere
int chunk_read(int base, int offset, int total, void** dest) {
	// 1. Calculate what is to be read from this block
	assert(base < N_BLOCKS);
	assert(offset < LEN_BLOCK); 

	int avail = LEN_BLOCK - offset; // This is how many bytes we can get out of this block
	int remaining = total - avail; // How much is left to read for next time
	int len;
	if (remaining <= 0) {
		remaining = 0;
		len = total; // We do not require all of the bytes in the block
	}
	else
		len = avail; // We would need more, but this is all we can get

	// 2. Allocate an internal buffer
	void* buf = malloc(LEN_BLOCK);
	assert(buf != NULL);

	// 3. Get a single block from disk
	int blocks_read = read_blocks(base, 1, buf);
	assert(blocks_read == 1);

	// 5. Copy the desired part of the block
	memcpy(*dest, buf + offset, len);

	// 6. Free the internal buffer
	free(buf);

	// 7. For next time, update the dest pointer to write to the correct place
	*dest += len;
	return remaining;
}


	

int struct_write(int base, int size, void* data, uint8_t* bitmap) {
	// 1. Calculate the number of blocks we must read to find this structure
	int blocks = (size + LEN_BLOCK - 1) / LEN_BLOCK; // Equivalent of ceil(size/LEN_BLOCK)

	// 1bis. Update the bitmap
	for (int i = base; i < base + blocks; i++) 
		force_set_index(i,bitmap);

	// 2. Allocate our internal buffer to hold what we write
	void* buf = malloc(blocks * LEN_BLOCK);
	assert(buf != NULL);

	// 3. Copy in the data
	memcpy(buf,data,size);

	// 4. Put the data on disk
	int blocks_written = write_blocks(base,blocks,buf);
	assert(blocks_written == blocks);

	// 5. Release our internal buffer
	free(buf);

	return 0;
}

// Writes a chunk of a block, avoiding trashing existing data, returning the bytes left to put somewhere else
int chunk_write(int base, int offset, int total, void** data) {
	// 1. Figure out how much of this block we'll write
	int avail = LEN_BLOCK - offset;
	int remaining = total - avail;
	int len;

	if (remaining <= 0) {
		len = total; // We do not write an entire block
		remaining = 0;
	}
	else
		len = avail; // The rest of the data must go elsewhere

	// 2. Allocate an internal buffer
	void* buf = malloc(LEN_BLOCK);
	assert(buf != NULL);

	// 3. Get a single block from disk
	int blocks_read = read_blocks(base, 1, buf);
	assert(blocks_read == 1);

	// 4. Splice in our data
	memcpy(buf + offset,*data, len);

	// 5. Write the block back
	int blocks_written = write_blocks(base, 1, buf);
	assert(blocks_written = 1);

	// 6. Release our internal buffer
	free(buf);

	// 7. For next time, update data and return how much must be placed elsewhere
	*data += len;
	return remaining;
}

