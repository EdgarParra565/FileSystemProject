/**
 * @file directory.h
 * @author Edgar Parra and Lucas Fuji
 *
 * Directory manipulation interface.
 * This layer manages directories, which are special types of files that contain
 * entries mapping names to inode numbers.
 */
#ifndef DIRECTORY_H
#define DIRECTORY_H

#define DIR_NAME_LENGTH 48

#include "blocks.h"
#include "inode.h"
#include "slist.h"

/**
 * Directory entry structure. 
 * This is the on-disk representation of a directory entry. 
 * It is 64 bytes long.
 */
typedef struct dirent {
  char name[DIR_NAME_LENGTH];
  int inum;
  char _reserved[12];
} dirent_t;

/**
 * Initializes the directory layer.
 * Should be called before any other directory functions.
 */
void directory_init();

/**
 * Looks up a file in a directory.
 * If the file is found, returns its inode number.
 * If the file is not found, returns -ENOENT.
 * 
 * @param di The inode of the directory to look up in.
 * @param name The name of the file to look up.
 * 
 * @return Integer is the inode number of the file on success,
 * and -ENOENT if the file does not exist.
 */
int directory_lookup(inode_t *di, const char *name);

/**
 * Puts an entry in a directory. 
 * If the entry already exists, it will be overwritten.
 * If the directory is full, returns -ENOENT.
 * 
 * @param di The inode of the directory to put the entry in.
 * @param name The name of the file to put.
 * @param inum The inode number of the file to put.
 * 
 * @return Integer is 0 on success, 
 * and -ENOENT if the directory is full.
 */
int directory_put(inode_t *di, const char *name, int inum);

/**
 * Deletes an entry from a directory.
 * If the entry does not exist, returns -ENOENT.
 * 
 * @param di The inode of the directory to delete the entry from.
 * @param name The name of the file to delete.
 * 
 * @return Integer is 0 on success, 
 * and -ENOENT if the file does not exist.
 */
int directory_delete(inode_t *di, const char *name);


#endif
