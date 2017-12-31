#ifndef _INCLUDE_GEOM_H
#define _INCLUDE_GEOM_H

#define FS_MAGIC 0xACBD0005
#define N_BLOCKS 1024
#define LEN_BLOCK 1024
#define N_INODES 100
#define N_DATA_PTRS 12
#define N_DIR_ENTRIES N_INODES
#define N_FD N_INODES
#define LEN_FILENAME 21 // To avoid an off-by-one error
#define LEN_FILENAME_EXT 3
#define LEN_BITMAP N_BLOCKS / 8


#endif // _INCLUDE_GEOM_H
