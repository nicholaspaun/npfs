#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>

#include "../sfs_api.h"
#include "dir.h"
#include "inode.h"

directory_entry* root;
int root_inode;



void dir_make(void) {
	// 1. Initialize the root directory structure
	directory_entry root[ino.n];
	for (int i = 0; i < ino.n; i++) {
		root[i].num = INODE_NONE;
	}

	// 2. Save it as a file
	int inum = inode_create(INODE_ROOT);
	assert(inum == 0); // On a fresh file system, there's no reason we should not have the first inode
	assert(file_write(inum,0,root,sizeof(root)) != -1); // Attempt to store the root directory
}

void dir_load(int root_ino) {
	// 1. Figure out if we are asked to reload the root directory from disk
	if (root_ino != -1)
		root_inode = root_ino;

	if (root != NULL)
		free(root);


	// Grab the root directory and store it
	root = malloc(ino.n * sizeof(directory_entry));
	file_read(root_inode,0,root,ino.n * sizeof(directory_entry));
}

int dir_add(char* path) {
	//1. Validate the path	
	int pathlen = strlen(path);


	if (pathlen >= LEN_FILENAME + (path[0] == '/')) {
	       //			^ fuse_wrappers handles filenames a bit oddly.
	       //			  I've tried to be helpful (see README).	
		
		print_error(120,path,"file name exceeds length limit");
		return -1;
	}
	else if (strcspn(path,".") != pathlen - LEN_FILENAME_EXT - 1) { // This enforces 16.3 convention
		print_error(121,path,"file name does not have a 3 character extension");
		return -1;
	}
		
	// 2. Find a slot
	
	int entry;
	for (entry = 0; entry < ino.n; entry++) {
		if (root[entry].num == -1)
			goto found;
	}

found:
	// 3. Put in the file and create an inode for it
	root[entry].num = inode_create(INODE_FILE);
	
	if (root[entry].num == INODE_FAIL) {
		print_error(130,"Disk is full","cannot create file");
		return -1;
	}

	strcpy(root[entry].name,path);
	assert(file_write(root_inode,0,root,ino.n * sizeof(directory_entry)) != -1); // Attempt to store the root directory

	return root[entry].num;
}

int dir_get(const char* path) {
	for (int i = 0; i < ino.n; i++) {
		if (root[i].num != INODE_NONE && !strcmp(root[i].name,path))
			return root[i].num;
	}
	return -1; // File not found -- may not be a problem
}

int dir_walk(char* path) {
	static int i = 0;


	for (; i < ino.n && root[i].num == INODE_NONE; i++); // Skip over blank entries

	if (i == ino.n) { // No more valid entries in this directory
		i = 0; // Reset directory pointer for next time
		return 0;
	}

	strcpy(path,root[i].name);
	i++;

	return 1;
}

int dir_remove(char* path) {
	for (int i = 0; i < ino.n; i++) {
		if (root[i].num != INODE_NONE && !strcmp(root[i].name,path)) {
			int inode = root[i].num;
			root[i].num = INODE_NONE;
			assert(file_write(root_inode,0,root,ino.n * sizeof(directory_entry)) != -1); // Attempt to store the root directory

			return inode;
		}
	}
	return -1;
}

void dir_dump(void) {
	for (int i = 0; i < ino.n; i++) {
		if (root[i].num != INODE_NONE)
			printf("%d: /%s\t%3d\n",i,root[i].name,root[i].num);
		else
			printf("%d: empty\n",i);
	}
}
