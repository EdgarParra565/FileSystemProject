/**
 * @file inode.h
 * @author Edgar Parra and Lucas Fuji
 *
 * Inode manipulation interface.
 * Defines the inode structure and provides routines to allocate, free, and
 * retrieve inodes by number.
 */
#ifndef INODE_H
#define INODE_H

#include "blocks.h"

/**
 * Inode structure. 
 * Representation of an inode. 
 * It should be exactly 16 bytes long.
 */
typedef struct inode {
  int refs;  // reference count
  int mode;  // permission & type
  int size;  // bytes
  int block; // single block pointer (if max file size <= 4K)
  int atime;
  int mtime;
} inode_t;

/**
 * Prints the details of an inode.
 * 
 * @param node The inode to print.
 * 
 * @return void
 */
void print_inode(inode_t *node);

/**
 * Retrieves a pointer to the inode with the specified inode number.
 * The inode number is the index of the inode in the table.
 * 
 * @param inum The inode number to retrieve.
 * 
 * @return A pointer to the requested inode, or NULL if not found.
 */
inode_t *get_inode(int inum);

/**
 * Allocates a new inode. 
 * If there are no free inodes, returns -1.
 * The allocated inode will be initialized to zero, 
 * with the block field set to -1.
 * 
 * @return Integer is the allocated inode number on success,
 * and -1 if there are no free inodes.
 */
int alloc_inode();

/**
 * Frees the specified inode.
 * If the inode has an associated block, that block will be freed also.
 * The inode will be reset to zero, and marked as free in the bitmap.
 * 
 * @param inum The inode number to free.
 * 
 * @return void
 */
void free_inode(int inum);


#endif
