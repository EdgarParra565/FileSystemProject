/**
 * @file directory.c
 * @author Edgar Parra and Lucas Fuji
 *
 * Implements the directory layer of the filesystem. This is the layer that
 * manages directories, which are special types of files that contain entries
 * mapping names to inode numbers.
 */
#include "directory.h"
#include "blocks.h"
#include "bitmap.h"
#include "inode.h"
#include <string.h>
#include <errno.h>

/**
 * Helper function to get the directory entries for a given inode.
 * If the inode does not have a block allocated, 
 * a new block will be allocated and initialized for the directory entries.
 * 
 * @param di The inode which will have acces to directory entires.
 * 
 * @return A pointer to the directory entries for the given inode.
 */
static dirent_t *get_dirents(inode_t *di) {
    if (di->block == -1) {
        di->block = alloc_block();
        dirent_t *input = blocks_get_block(di->block);
        for (int i = 0; i < 64; i++) {
            input[i].inum = -1;
        }
    }
    return blocks_get_block(di->block);
}

void directory_init() {}

int directory_lookup(inode_t *di, const char *name) {
    dirent_t *input = get_dirents(di);
    for (int i = 0; i < 64; i++) {
        if (input[i].inum != -1 && strcmp(input[i].name, name) == 0) {
            return input[i].inum;
        }
    }
    return -ENOENT;
}

int directory_put(inode_t *di, const char *name, int inum) {
    dirent_t *input = get_dirents(di);
    for (int i = 0; i < 64; i++) {
        if (input[i].inum == -1) {
            strncpy(input[i].name, name, DIR_NAME_LENGTH - 1);
            input[i].name[DIR_NAME_LENGTH - 1] = '\0';
            input[i].inum = inum;
            return 0;
            
        }
    }
    return -ENOENT;
}

int directory_delete(inode_t *di, const char *name) {
    dirent_t *input = get_dirents(di);
    for (int i = 0; i < 64; i++) {
        if (input[i].inum != -1 && strcmp(input[i].name, name) == 0) {
            input[i].inum = -1;
            input[i].name[0] = '\0';
            return 0;
        } 
    }
    return -ENOENT;
}

