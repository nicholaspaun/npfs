#ifndef _INCLUDE_FILEDES_H_
#define _INCLUDE_FILEDES_H_

/* filedes_init(): Intializae a new file descriptor table */
void filedes_init(void);

/* int fd = filedes_open(uint64_t inodeIndex): Assign a new FD for a file commencing at that inode
 * Errors (-1): opening the same file twice; full file descriptor table 
 */
int filedes_open(uint64_t inodeIndex);

/* int error = filedes_close(int fd): Close fd
 * Error (-1): file descriptor was not open
 */
int filedes_close(int fd);

/* filedes_seek(int fd, int pos): Seek fd to pos bytes
 * Assertion: pos e [0,file length]
 */
int filedes_seek(int fd, int pos);

/* int pos = filedes_tell(int fd): Return position in bytes of fd's read/write pointer
 */

int filedes_tell(int fd);

/* int inode = fildes_inode(int fd): Returns inode associated with file descriptor
 */
int filedes_inode(int fd);

#endif // _INCLUDE_FILEDES_H_
