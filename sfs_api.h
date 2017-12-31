#ifndef _INCLUDE_SFS_API_H_
#define _INCLUDE_SFS_API_H_

#include <stdint.h>
#include "src/geom.h"

// The following two constants are not used
#define MAX_FILE_NAME 20
#define MAX_EXTENSION_NAME 3

typedef struct superblock_t {
    uint64_t magic;
    uint64_t block_size;
    uint64_t fs_size;
    uint64_t inode_table_len;
    uint64_t root_dir_inode;
} superblock_t;

typedef struct inode_t {
    unsigned int mode;
    unsigned int link_cnt;
    unsigned int uid;
    unsigned int gid;
    unsigned int size;
    unsigned int data_ptrs[N_DATA_PTRS];
    unsigned int indirectPointer; // points to a data block that points to other data blocks (Single indirect)
} inode_t;

/*
 * inodeIndex   which inode this entry describes
 * inode  	pointer towards the inode in the inode table
 * rwptr    	where in the file to start   
 */
typedef struct file_descriptor {
    uint64_t inodeIndex;
    inode_t* inode;  
    uint64_t rwptr;
} file_descriptor;


typedef struct directory_entry {
    int num; // inode number of the entry. 
    char name[LEN_FILENAME + 1]; // fuse_wrappers handles filenames a bit oddly -- I've tried to be helpful (see README)
} directory_entry;


void mksfs(int fresh);
int sfs_getnextfilename(char *fname);
int sfs_getfilesize(const char* path);
int sfs_fopen(char *name);
int sfs_fclose(int fileID);
int sfs_fread(int fileID, char *buf, int length);
int sfs_fwrite(int fileID, const char *buf, int length);
int sfs_fseek(int fileID, int loc);
int sfs_remove(char *file);


void print_error(int code, const char* blame, const char* message);
#endif //_INCLUDE_SFS_API_H_
