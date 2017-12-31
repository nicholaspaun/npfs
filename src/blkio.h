#ifndef _INCLUDE_BLKIO_H_
#define _INCLUDE_BLKIO_H_

/* struct_read: Read a contiguous, size byte data structure, commencing at block base
 * dest will point to a pointer to the memory location used to store the data.
 * If dest points to NULL, we will allocate it
 */
int struct_read(int base, int size, void** dest);

/* chunk_read: Read a discontiguous object, total bytes remaining to read, commencing at byte offset of block base.
 * dest will point to a pointer. This pointer is set to the right location to continue reading at the next block.
 * Returns number of bytes remaining to be found in another block
 */
int chunk_read(int base, int offset, int total, void** dest);

/* struct_write: Write a contiguous, size byte data structure, commencing at block base,
 * recording the necessary allocation in a free bit map called bitmap
 * Data is taken from data.
 */
int struct_write(int base, int size, void* data,uint8_t* bitmap);


/* chunk_write: Write a discontiguous object, total bytes remaining to be written, commencing at byte offset of block base.
 * data will point to a pointer. This pointer is set to the right location to continue writing at the next block.
 * Returns number of bytes remaining to write to another block
 */
int chunk_write(int base, int offset, int total, void** data);

#endif // _INCLUDE_BLKIO_H_
