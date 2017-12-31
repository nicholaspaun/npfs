#ifndef _INCLUDE_INODE_H_
#define _INCLUDE_INODE_H_

#define INODE_FAIL -1 // Failure to assign an inode or disk block
#define INODE_NONE -1 // Unused inode
#define INODE_INDIRECT 0 // File continuation
#define INODE_FILE 1 // Ordinary file
#define INODE_ROOT 3 // The root directory

typedef struct {
	int n;
	inode_t* s;
} inode_table;

extern inode_table ino;
extern uint8_t* free_bit_map;


/* inode_load(int inode_table_len): Load an inode table containing the calculated number of inodes */

void inode_load(int inode_table_len);

/* inode_make_table(): Creates a new inode table and saves it to disk */
void inode_make_table(uint8_t* initial_bit_map);

/* int inode = inode_create(int type): Create an inode of requested type
 * Returns # of new inode of INODE_FAIL
 */
int inode_create(int type);

/* int bytes_read = file_read(int inode, int offset, void* dest, int length);
 * From position offset, read length bytes into pointer dest,
 * from the file commencing at inode inode.
 *
 * Returns bytes_read or -1 on serious error.
 */
int file_read(int inode, int offset, void* dest, int length);

/* int bytes_written = file_write(int inode, int offset, void* data, int length);
 * From position offset, write length bytes from pointer dest,
 * to the file commencing at inode inode
 *
 * Returns bytes_written or -1 on serious error (full disk usually)
 */
int file_write(int inode, int offset, const void* data, int length);

/* inode_dump(): Show the inode table for debugging purposes */
void inode_dump(void);

/* file_remove(int inode): Remove the file that starts at inode */
void file_remove(int inode);

/* int size = file_size(int inode): Get the size of the file starting at inode */
int file_size(int inode);
#endif // _INCLUDE_INODE_H_
