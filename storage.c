/**
 * @file storage.c
 * @author Edgar Parra and Lucas Fuji
 *
 * Implements the storage layer of the filesystem. This is the layer that
 * directly manipulates the disk image and provides an interface for the
 * higher layers to interact with the storage object.
 */
#include "storage.h"
#include "inode.h"
#include "blocks.h"
#include "bitmap.h"
#include "directory.h"
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>


/**
 * Helper function to get the inode number for a given path.
 * 
 * @param path Path to the file. The function will traverse the directory
 * structure to find the inode number corresponding to the path.
 * 
 * @return Integer is the inode number on success, and -ENOENT if the file does not exist.
 */
static int inumPath(const char *path) {
    if (strcmp(path, "/") == 0) return 0;
    slist_t *components = slist_explode(path + 1, '/');
    int cur_inum = 0;
    for (slist_t *comp = components; comp != NULL; comp = comp->next) {
        inode_t *node = get_inode(cur_inum);
        if (!(node->mode & 040000)) {
            slist_free(components);
            return -ENOTDIR;
        }
        cur_inum = directory_lookup(node, comp->data);
        if (cur_inum < 0) {
            slist_free(components);
            return -ENOENT;
        }
    }
    slist_free(components);
    return cur_inum;
}

/**
 * Helper function to get the parent inode number and the name of the file.
 * 
 * @param path Path to the file. The function will extract the parent directory
 * and the name of the file from the path.
 * @param name_out Buffer to store the name of the file. The buffer should be
 * at least DIR_NAME_LENGTH bytes long.
 * 
 * @return Integer is 0 on success, and -ENOENT if the file does not exist.
 */
static int get_parent_inum(const char *path, char *name_out) {
    char temp[256];
    strncpy(temp, path, 255);
    temp[255] = '\0';
    char *lastIndex = strrchr(temp, '/');
    strncpy(name_out, lastIndex + 1, DIR_NAME_LENGTH -1);
    name_out[DIR_NAME_LENGTH -1] = '\0';
    *lastIndex = '\0';
    if (strlen(temp) == 0) {
        return 0;
    }
    return inumPath(temp);
}

void storage_init(const char *path) {
    blocks_init(path);
    bitmap_put(get_blocks_bitmap(), 1, 1);
    inode_t *root = get_inode(0);
    if (root->mode == 0) {
        bitmap_put(get_inode_bitmap(), 0, 1);
        root->refs  = 1;
        root->mode  = 040755;
        root->size  = 0;
        root->block = -1;
        int bnum = alloc_block();
        root->block = bnum;
        dirent_t *entries = blocks_get_block(bnum);
        for (int i = 0; i < 64; i++) {
            entries[i].inum = -1;
        }
    }
}

int storage_stat(const char *path, struct stat *st) {
    int inum = inumPath(path);
    if (inum < 0) {
        return -ENOENT;
    }
    inode_t *node = get_inode(inum);
    st->st_mode  = node->mode;
    st->st_size  = node->size;
    st->st_nlink = node->refs;
    st->st_uid   = getuid();
    st->st_atime = node->atime;
    st->st_mtime = node->mtime;
    return 0;
}

int storage_mknod(const char *path, int mode) {
    char name[DIR_NAME_LENGTH];
    int par_inum = get_parent_inum(path, name);
    inode_t *par = get_inode(par_inum);
    int existing = directory_lookup(par, name);
    if (existing >= 0) return -EEXIST;
    int new_inum = alloc_inode();
    if (new_inum == -1) return -ENOSPC;
    inode_t *node = get_inode(new_inum);
    node->mode = mode;
    node->refs = 1;
    node->size = 0;
    int rv = directory_put(par, name, new_inum);
    return rv;
}

int storage_truncate(const char *path, off_t size) {
    int inum = inumPath(path);
    if (inum < 0) {
        return -ENOENT;
    }
    inode_t *node = get_inode(inum);
    if (size == 0) {
        if (node->block != -1) {
            free_block(node->block);
            node->block = -1;
        }
        node->size = 0;
    }
    return 0;
}

int storage_write(const char *path, const char *buf, size_t size, off_t offset) {
    int inum = inumPath(path);
    if (inum < 0) {
        return -ENOENT;
    }
    inode_t *node = get_inode(inum);
    if (node->block == -1) {
        node->block = alloc_block();
    }
    char *data = blocks_get_block(node->block);
    memcpy(data + offset, buf, size);
    if (offset + size > node->size) {
        node->size = offset + size;
    }
    return size;
}

int storage_read(const char *path, char *buf, size_t size, off_t offset) {
    int inum = inumPath(path);
    if (inum < 0) {
        return -ENOENT;
    }
    inode_t *node = get_inode(inum);
    if (node->block == -1) {
        return 0;
    }
    if (offset >= node->size) {
        return 0;
    }
    if (offset + size > node->size) {
        size = node->size - offset;
    }
    char *data = blocks_get_block(node->block);
    memcpy(buf, data + offset, size);
    return size;
}

slist_t *storage_list(const char *path) {
    int inum = inumPath(path);
    if (inum < 0) {
        return NULL;
    }
    inode_t *node = get_inode(inum);
    if (node->block == -1) {
        return NULL;
    }
    dirent_t *entries = blocks_get_block(node->block);
    slist_t *res = NULL;
    for (int i = 0; i < 64; i++) {
        if (entries[i].inum == -1) {
            continue;
        }
        if (strcmp(entries[i].name, ".") == 0) {
            continue;
        }
        if (strcmp(entries[i].name, "..") == 0) {
            continue;
        }
        res = slist_cons(entries[i].name, res);
    }
    return res;
}

int storage_unlink(const char *path) {
    char name[DIR_NAME_LENGTH];
    int parent = get_parent_inum(path, name);
    if (parent < 0) {
        return -ENOENT;
    }
    inode_t *par = get_inode(parent);
    int inum = directory_lookup(par, name);
    if (inum < 0) {
        return -ENOENT;
    }
    free_inode(inum);
    directory_delete(par, name);
    return 0;
}

int storage_rename(const char *from, const char *to) {
    char fromName[DIR_NAME_LENGTH];
    char toName[DIR_NAME_LENGTH];
    int fromParent = get_parent_inum(from, fromName);
    int toParent = get_parent_inum(to, toName);
    if (fromParent < 0 || toParent < 0) {
        return -ENOENT;
    }
    inode_t *fromParentNode = get_inode(fromParent);
    int inum = directory_lookup(fromParentNode, fromName);
    if (inum < 0) {
        return -ENOENT;
    }
    inode_t *toParentNode = get_inode(toParent);
    directory_put(toParentNode, toName, inum);
    directory_delete(fromParentNode, fromName);
    return 0;
}

int storage_set_time(const char *path, const struct timespec ts[2]) {
    int inum = inumPath(path);
    if (inum < 0) {
        return -ENOENT;
    }
    inode_t *node = get_inode(inum);
    node->atime = (int)ts[0].tv_sec;
    node->mtime = (int)ts[1].tv_sec;
    return 0;
}

int storage_mkdir(const char *path, mode_t mode) {
    char name[DIR_NAME_LENGTH];
    int parent = get_parent_inum(path, name);
    if (parent < 0) {
        return -ENOENT;
    }
    inode_t *parent2 = get_inode(parent);
    if (directory_lookup(parent2, name) >= 0) {
        return -EEXIST;
    }
    int inum = alloc_inode();
    if (inum == -1) {
        return -ENOSPC;
    }
    inode_t *node = get_inode(inum);
    node->mode= mode | 040000;
    node->refs = 1;
    node->size = 0;
    node->block = -1;
    directory_put(node, ".", inum);
    directory_put(node, "..", parent);
    return directory_put(parent2, name, inum);
}

int storage_rmdir(const char *path) {
    char name[DIR_NAME_LENGTH];
    int parent = get_parent_inum(path, name);
    if (parent < 0) {
        return -ENOENT;
    }
    int inum = inumPath(path);
    if (inum < 0) {
        return -ENOENT;
    }
    inode_t *node = get_inode(inum);
    if (node->block == -1) {
        inode_t *parent2 = get_inode(parent);
        directory_delete(parent2, name);
        free_inode(inum);
        return 0;
    }
    dirent_t *input = (dirent_t *)blocks_get_block(node->block);
    for (int i = 0; i < 64; i++) {
        if (input[i].inum == -1) continue;
        if (strcmp(input[i].name, ".") == 0) continue;
        if (strcmp(input[i].name, "..") == 0) continue;
        return -ENOTEMPTY;
    }
    inode_t *parent2 = get_inode(parent);
    directory_delete(parent2, name);
    free_inode(inum);
    return 0;
}