// Disk storage abstracttion.
//
// Feel free to use as inspiration. Provided as-is.

// based on cs3650 starter code

#ifndef NUFS_STORAGE_H
#define NUFS_STORAGE_H

#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include "slist.h"

/**
 * Initializes the storage object. 
 * Should be called before any other storage functions.
 * 
 * @param path Path to the file. If the file does not exits, 
 * it will be created and initialized as empty storage.
 * 
 * @return void
 */
void storage_init(const char *path);

/**
 * Gets the current status of the storage object. 
 * Can be called on any path that exists in the storage.
 * 
 * @param path Path to the file. If the file does not exits,
 * the function returns -ENOENT.
 * @param st Pointer to the stat structure to fill in with the file's attributes.
 * 
 * @return Integer is 0 on success, and -ENOENT if the file does not exist.
 */
int storage_stat(const char *path, struct stat *st);

/**
 * Reads data from a file in the storage object.
 * Can be called on any path that exists in the storage.
 * 
 * @param path Path to the file. If the file does not exits, 
 * the function returns -ENOENT.
 * @param buf Buffer to store the read data.
 * @param size Number of bytes to read.
 * @param offset Offset in the file to start reading from.
 * 
 * @return Integer is 0 on success, and -ENOENT if the file does not exist.
 */
int storage_read(const char *path, char *buf, size_t size, off_t offset);


/**
 * Writes data to a file in the storage object.
 * Can be called on any path that exists in the storage.
 * 
 * @param path Path to the file. If the file does not exits, 
 * the function returns -ENOENT.
 * @param buf Buffer containing the data to write.
 * @param size Number of bytes to write.
 * @param offset Offset in the file to start writing to.
 * 
 * @return Integer is 0 on success, and -ENOENT if the file does not exist.
 */
int storage_write(const char *path, const char *buf, size_t size, off_t offset);


/**
 * Truncates (resizing to a specific size) a file in the storage object.
 * Can be called on any path that exists in the storage.
 * 
 * @param path Path to the file. If the file does not exits, 
 * the function returns -ENOENT.
 * @param size New size of the file.
 * 
 * @return Integer is 0 on success, and -ENOENT if the file does not exist.
 */
int storage_truncate(const char *path, off_t size);

/**
 * Creates a new file or device node in the storage object.
 * 
 * @param path Path to the file. If the file already exists, 
 * the function returns -ENOENT.
 * @param mode Mode for the new file or device node.
 * 
 * @return Integer is 0 on success, and -ENOENT if the file does not exist.
 */
int storage_mknod(const char *path, int mode);

/**
 * Deletes a file from the storage object.
 * Can be called on any path that exists in the storage.
 * 
 * @param path Path to the file. If the file does not exist, 
 * the function returns -ENOENT.
 * 
 * @return Integer is 0 on success, and -ENOENT if the file does not exist.
 */
int storage_unlink(const char *path);

/**
 * Renames a file in the storage object.
 * Can be called on any path that exists in the storage.
 * 
 * @param from Path to the existing file. If the file does not exist, 
 * the function returns -ENOENT.
 * @param to Path to the new name. If the file already exists, 
 * the function returns -ENOENT.
 * 
 * @return Integer is 0 on success, and -ENOENT if the file does not exist.
 */
int storage_rename(const char *from, const char *to);

/**
 * Updates the integer count on a given file in the storage object for naming/identification purposes.
 * Can be called on any path that exists in the storage.
 * 
 * @param path Path to the file. If the file does not exist, 
 * the function returns -ENOENT.
 * @param ts Array of two timespec structures containing the new access and modification times.
 * 
 * @return Integer is 0 on success, and -ENOENT if the file does not exist.
 */
int storage_set_time(const char *path, const struct timespec ts[2]);

/**
 * Lists the entries in the storage object.
 * Can be called on any path that exists in the storage.
 * 
 * @param path Path to the directory. If the directory does not exist, 
 * the function returns NULL.
 * 
 * @return A pointer to a linked list or Null if the directory does not exist.
 */
slist_t *storage_list(const char *path);

/**
 * Creates a new empty directory in the storage object.
 * Can be called on any path that exists in the storage.
 * 
 * @param path Path to the directory. If the directory already exists,
 * the function returns -ENOENT.
 * @param mode of type mode_t, specifices the permissions for the directory.
 * 
 * @return Integer is 0 on success, and -ENOENT if the directory already exists.
 */
int storage_mkdir(const char *path, mode_t mode);

/**
 * Removes a directory from the storage object.
 * Can be called on any path that exists in the storage.
 * 
 * @param path Path to the directory. If the directory does not exist, 
 * the function returns -ENOENT.
 * 
 * @return Integer is 0 on success, and -ENOENT if the directory does not exist.
 */
int storage_rmdir(const char *path);


#endif
