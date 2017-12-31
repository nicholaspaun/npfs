
#include "sfs_api.h"
#include "bitmap.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fuse.h>
#include <strings.h>
#include "disk_emu.h"
#include "src/geom.h"
#include "src/inode.h"
#include "src/filedes.h"
#include "src/dir.h"
#include "src/fs.h"

#define PAUN_NICHOLAS_DISK "paun_nicholas_disk.disk"
#define NUM_BLOCKS 1024  // Maximum number of data blocks on the disk.
#define BITMAP_ROW_SIZE (NUM_BLOCKS/8) // This essentially mimcs the number of rows we have in the bitmap. we will have 128 rows. 

void print_error(int code, const char* blame, const char* message) {
	fprintf(stderr,"\033[1;31m[ERROR %3d]\033[0m %s: %s.\n",code,blame,message);
}

void mksfs(int fresh) {
	if (fresh) mkfs(PAUN_NICHOLAS_DISK);
	fs_mount(PAUN_NICHOLAS_DISK);
	if (fresh) {dir_make(); dir_load(-1);} // The root directory is created like a file, once the FS is mounted


}
int sfs_getnextfilename(char *fname){
	return dir_walk(fname);

}
int sfs_getfilesize(const char* path){
	int inode = dir_get(path);
	if (inode == INODE_FAIL) {
		print_error(100,path,"file not found");
		return -1;
	}

	return file_size(inode);
}

int sfs_fopen(char *name){
	int inode = dir_get(name);
	if (inode == -1) { // File not found
		// Create file
		inode = dir_add(name);
		if (inode == INODE_FAIL) 
			return -1;
		
		}

	return filedes_open(inode);
}

int sfs_fclose(int fileID) {
	return filedes_close(fileID);
}
int sfs_fread(int fileID, char *buf, int length) {
	int inode = filedes_inode(fileID);

	if (inode == -1)
		return -1;
	

	int pos = filedes_tell(fileID);

	int bytes_read = file_read(inode,pos,buf,length);
	filedes_seek(fileID,pos + bytes_read);
	return bytes_read;
	
}
int sfs_fwrite(int fileID, const char *buf, int length) {
	int inode = filedes_inode(fileID);

	if (inode == -1)
		return -1;


	int pos = filedes_tell(fileID);

	int bytes_written = file_write(inode,pos,buf,length);
	filedes_seek(fileID,pos + bytes_written);
	return bytes_written;
}
int sfs_fseek(int fileID, int loc) {
	return filedes_seek(fileID,loc);
}

int sfs_remove(char *file) {
	int inode = dir_remove(file);
	if (inode == -1) {
		print_error(101,file,"file not found");
		return -1;
	}
	file_remove(inode);
	return 0;
}

