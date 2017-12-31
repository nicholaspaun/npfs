#ifndef _INCLUDE_DIR_H_
#define _INCLUDE_DIR_H_

/* dir_make(): Initialize an empty root directory */
void dir_make(void);

/* dir_load(int root_inode): Load root directory from disk (located at root_inode) */
void dir_load(int root_inode);

/* int assigned_inode = dir_add(path): Create a new file
 * Errors (-1): invalid filename; full disk
 */
int dir_add(char* path); 

/* int file_inode = dir_get(char* path): Retrive inode assigned to a file
 * Error (-1): file not found
 */
int dir_get(const char* path);

/* int more_files = dir_walk(char* path): Get next file in directory and place it in path
 * Returns 0 when traverse is complete */
int dir_walk(char* path);

/* int error = dir_remove(char* path): Removes file from directory only
 * Error (-1): file not found */
int dir_remove(char* path);

#endif //_INCLUDE_DIR_H_
