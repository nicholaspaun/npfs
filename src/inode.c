#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "../sfs_api.h"
#include "../bitmap.h"
#include "blkio.h"
#include "inode.h"

inode_table ino = {.n = 0, .s = NULL};
uint8_t* free_bit_map = NULL;


void inode_make_table(uint8_t* initial_bitmap) {
	inode_t ino[N_INODES];
	for (int i = 0; i < N_INODES; i++) // Mark each inode as unused
		ino[i].link_cnt = INODE_NONE;

	struct_write(1,sizeof(ino),&ino,initial_bitmap);
}

void inode_load(int inode_table_len) {
	if (ino.s != NULL) {// Figure out if we are re-initializing
		free(ino.s);
		ino.s = NULL;
		free(free_bit_map);
		free_bit_map = NULL;
	}

	ino.n = inode_table_len;
	struct_read(1,ino.n * sizeof(inode_t),(void**) &(ino.s));
	struct_read(N_BLOCKS - 1,LEN_BITMAP,(void**) &free_bit_map);
}
	


int inode_create(int type) {
	int inum;
	inode_t* this;
	for (inum = 0; inum < ino.n; inum++) {
		if (ino.s[inum].link_cnt == INODE_NONE) {
			goto found;
		}
	}

	return INODE_FAIL;


found: 
	this = &(ino.s[inum]);

	this->mode = 0; 
	this->link_cnt = type;
	this->uid = 0;
	this->gid = 0;
	this->size = 0;

	for (int i = 0; i < N_DATA_PTRS; i++)
		this->data_ptrs[i] = INODE_NONE;

	this->indirectPointer = INODE_NONE;

	return inum;
}

int file_read(int inum, int offset, void* dest, int length) {

	// 1. Reject obviously improper requests
	assert(inum >= 0 && inum < ino.n);

	inode_t* this = &ino.s[inum];
	assert(this->link_cnt > INODE_INDIRECT);
	assert(offset >= 0 && offset <= this->size);
	assert(length >= 0);

	// 2. Figure out where in the file we are to start reading
	
	// It is not possible to read past the end of file, so we clamp all reads to end there if they're longer
	length = length < this->size? length : this->size;

	int block = offset / LEN_BLOCK;
	int startByte = offset % LEN_BLOCK;
	int startInode = block / N_DATA_PTRS;
	int startDataPtr = block % N_DATA_PTRS;


	// 3. Advance to the right point
	
	// 3.1 Follow single indirect inode links to get to the right place
	int indirection = 0;
	while (indirection < startInode) {
		assert(this->indirectPointer != INODE_NONE); // Corrupted inode
		this = &ino.s[this->indirectPointer]; // Continue down the path
		indirection++;
	}

	// 4. Begin reading
	int remaining = length;
	void** pos = &dest; // Will keep track of how much of the file we've loaded so far
	for (;;) {
		for (int i = startDataPtr; i < N_DATA_PTRS; i++) { // Proceed data block by data block

			if (this->data_ptrs[i] == INODE_NONE) { // Early EOF
				goto end;
			}
				
			remaining = chunk_read(this->data_ptrs[i],startByte,remaining,pos);
			if (remaining == 0) // Actually read to the desired length
				goto end;
			startByte = 0; // Next time, we start at the front of the block
		}

		// Advance to the next inode
		if (this->indirectPointer == INODE_NONE) {  // Early EOF
			goto end;
		}
		
		this = &ino.s[this->indirectPointer];
		startDataPtr = 0; // Next time start at the front of the inode
	}

end:
	return length - remaining;
}


int file_write(int inum, int offset, const void* data, int length) {
	// 1. Reject obviously invalid requests
	assert(inum >= 0 && inum < ino.n);

	inode_t* this = &ino.s[inum];
	assert(this->link_cnt > INODE_INDIRECT);

	assert(offset >= 0 && offset <= this->size);
	assert(length >= 0);

	// 2. Figure out where in the file we are to start writing

	int block = offset / LEN_BLOCK;
	int startByte = offset % LEN_BLOCK;
	int startInode = block / N_DATA_PTRS;
	int startDataPtr = block % N_DATA_PTRS;


	// 3. Advance to the right point
	
	// 3.1 Follow single indirect inode links to get to the right place
	int indirection = 0;
	while (indirection < startInode) {
		if (this->indirectPointer == INODE_NONE) {
			this->indirectPointer = inode_create(INODE_INDIRECT); // You can grow a file by seeking past its end
			if (this->indirectPointer == INODE_FAIL) { // No room for file
				print_error(171,"Cannot grow file","inode table is full");
				return 0; // We did not even start to write
			}
				
		}

		this = &ino.s[this->indirectPointer]; // Continue down the path
		indirection++;
	}


	// 4. Begin writing
	int remaining = length;
	void** pos = (void**) &data;
	for (;;) {
		for (int i = startDataPtr; i < N_DATA_PTRS; i++) { // Proceed data block by data block
			if (this->data_ptrs[i] == INODE_NONE) {
				this->data_ptrs[i] = get_index(free_bit_map); // Reserve ourselves a new block
				if (this->data_ptrs[i]  == INODE_FAIL) { // Early full disk
					print_error(180,"Cannot grow file","all blocks are used");
					goto commit;
				}
			}
			remaining = chunk_write(this->data_ptrs[i],startByte,remaining,pos);
			if (remaining == 0) // File correctly written
				goto commit;
			startByte = 0; // Next time, we start at the front of the block
		}

		// Indirection is required to continue the data block
		if (this->indirectPointer == INODE_NONE) {
			this->indirectPointer = inode_create(INODE_INDIRECT); // Single indirect inode to continue this file
			if (this->indirectPointer == INODE_FAIL) { // Early full disk
				print_error(173,"Cannot grow file","inode table is full");
				goto commit;
			}
		}
		this = &ino.s[this->indirectPointer];
		startDataPtr = 0; // Next time start at the front of the inode
	}

	return -1;
	int growth;
commit:

	// Recalculation of the file size works because our files cannot shrink
	growth = -ino.s[inum].size + offset + (length - remaining);	
       	ino.s[inum].size += (growth > 0 ? growth : 0);


	// Save changes to disk
	struct_write(1,ino.n * sizeof(inode_t),ino.s,free_bit_map); // Save inode table
	struct_write(N_BLOCKS - 1, LEN_BITMAP, free_bit_map, free_bit_map); // Save bitmap
	return length - remaining;
}


void inode_dump(void) {
	inode_t* this;
	for (int i = 0; i < ino.n; i++) {
		this = &ino.s[i];

		printf("%3d (%1d) %6d bytes\t[%3d] [%3d] [%3d] . [%3d] [%3d] [%3d] . [%3d] [%3d] [%3d] . [%3d] [%3d] [%3d] . {%d}\n",
			i, this->link_cnt, this->size,
			this->data_ptrs[0], this->data_ptrs[1], this->data_ptrs[2], this->data_ptrs[3],
			this->data_ptrs[4], this->data_ptrs[5], this->data_ptrs[6], this->data_ptrs[7],
			this->data_ptrs[8], this->data_ptrs[9], this->data_ptrs[10], this->data_ptrs[11],
			this->indirectPointer
		);
	}
}

void file_remove(int inum) {
	assert(inum >= 0 && ino.n);
	inode_t* this = &ino.s[inum];
	assert(this->link_cnt > INODE_INDIRECT);

	while (inum != -1) {
		this = &ino.s[inum];
		this->link_cnt = INODE_NONE;

		for (int i = 0; i < N_DATA_PTRS; i++) {
			if (this->data_ptrs[i] == INODE_NONE) break;
			rm_index(this->data_ptrs[i],free_bit_map); // De-allocate the block
		}


		inum = this->indirectPointer;
	}

	struct_write(1,ino.n * sizeof(inode_t),ino.s,free_bit_map);
	struct_write(N_BLOCKS - 1, LEN_BITMAP, free_bit_map, free_bit_map);
}

int file_size(int inum) {
	assert(inum >= 0 && ino.n);
	inode_t* this = &ino.s[inum];
	assert(this->link_cnt > INODE_INDIRECT);

	return this->size;
}
