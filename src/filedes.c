#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "../sfs_api.h"
#include "filedes.h"
#include "inode.h"

file_descriptor* fdt = NULL;

int check_fd(int fd) {
	if (fd < 0 || fd > ino.n) 
		print_error(210,"check_fd","Attempt to operate on an invalid file descriptor. Check your tester code");
	
	else if (fdt[fd].inode == NULL) 
		print_error(220,"check_fd","Attempt to operate on a closed or nonexistent file descriptor");
	
	else
		return 1;

	return 0;
}

void filedes_init(void) {
	if (fdt != NULL) { // Re-initialization
		free(fdt);
	}

	fdt = malloc(sizeof(file_descriptor) * ino.n);
	for (int i = 0; i < ino.n; i++) {
		fdt[i] = (file_descriptor) {
			.inodeIndex = -1,
			.inode = NULL,
			.rwptr = -1
		};
	};
}

int filedes_open(uint64_t inodeIndex) {
	// 1. Make sure we have a valid inode
	assert(inodeIndex >= 0 && inodeIndex < ino.n);
	inode_t* node = &ino.s[inodeIndex];
	assert(node->link_cnt > 0);

	// 1bis. Apparently we should not allow the same file to be opened twice
	for (int i = 0; i < ino.n; i++) {
		 if (fdt[i].inodeIndex == inodeIndex) {
			 print_error(140,"Opening the same file twice","not permitted");
			 return -1;
		}
	}
	

	// 2. Find a slot in the FDT
	int i;
	for (i = 0; i < ino.n; i++) {
		if (fdt[i].inode == NULL) 
			goto found;
	}

	print_error(150,"File descriptor table","full");
	return -1;

found:
	//3. Fill out an entry
	fdt[i].inodeIndex = inodeIndex;
	fdt[i].inode = node;
	fdt[i].rwptr = node->size; // Weirdly, the RW pointer begins at EOF
	return i;

}

int  filedes_close(int fd) {
	if (!check_fd(fd)) return -1;

	fdt[fd].inodeIndex = -1;
	fdt[fd].inode = NULL;

	return 0;
}

int filedes_seek(int fd, int pos) {
	if (!check_fd(fd)) return -1;
	if (pos < 0 || pos > fdt[fd].inode->size) {
		print_error(200,"Seek postion","negative or exceeds file length");
		return -1;
	}

	assert(pos >= 0 && pos <= fdt[fd].inode->size);
	fdt[fd].rwptr = pos;
	return pos;
}

int filedes_tell(int fd) {
	if (!check_fd(fd)) return -1;
	return fdt[fd].rwptr;
}

int filedes_inode(int fd) {
	if (!check_fd(fd)) return -1;

	return fdt[fd].inodeIndex;
}


