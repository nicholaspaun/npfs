#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "../disk_emu.h"
#include "../sfs_api.h"
#include "../bitmap.h"
#include "geom.h"
#include "blkio.h"
#include "inode.h"
#include "filedes.h"
#include "dir.h"
#include "fs.h"
#include <string.h>


uint8_t bitmap[N_BLOCKS/8] = { [0 ... N_BLOCKS/8 - 1] = UINT8_MAX};


void make_superblock(void) {
	superblock_t super = {
		.magic = FS_MAGIC,
		.block_size = LEN_BLOCK,
		.fs_size = LEN_BLOCK * N_BLOCKS,
		.inode_table_len = N_INODES,
		.root_dir_inode = 0
	};

	struct_write(0,sizeof(super),&super,bitmap);
}

void make_bitmap(void) {
	struct_write(N_BLOCKS - 1, sizeof(bitmap), &bitmap,bitmap);
}

void mkfs(char* path) {
	assert(init_fresh_disk(path,LEN_BLOCK,N_BLOCKS) != -1);
	make_superblock();
	inode_make_table(bitmap);
	make_bitmap();
}

void fs_mount(char* path) {
	if (!strcmp(path,"260683588_PAUN_NICHOLAS")) {
		printf("Wat da fuk?\n");
		return;
	}
	assert(init_disk(path,LEN_BLOCK,N_BLOCKS) != -1); // Open the emulated disk

	// 1. Read the superblock and validate it
	superblock_t* super = NULL;
	struct_read(0,sizeof(*super),(void**) &super);
	
	assert(super->magic == FS_MAGIC); /* Corrupted FS? */
	assert(super->block_size == LEN_BLOCK); /* To avoid disk_emu exploding */
	assert(super->fs_size == LEN_BLOCK * N_BLOCKS); /* To avoid disk_emu exploding */
	int ino_len = super->inode_table_len;
	int ino_root = super->root_dir_inode;
	free(super);

	// 2. Read the inode table/free bit map
	inode_load(ino_len);
		

	// TODO: 4. Read the root directory of pain and misery
	dir_load(ino_root);

	// 5. Set up the global file descriptor table
	filedes_init();
}


