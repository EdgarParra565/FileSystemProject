/**
 * @file inode.c
 * @author Edgar Parra and Lucas Fuji
 *
 * Implements the inode management for the NUFS filesystem. Includes
 * methods that allocates and frees inodes, as well as to get an inode by its
 * number.
 */
#include "inode.h"
#include "blocks.h"
#include "bitmap.h"
#include <string.h>
#include <stdio.h>

void print_inode(inode_t *node) {
  printf("inode { refs=%d, mode=%04o, size=%d, block=%d }\n",
    node->refs, node->mode, node->size, node->block);
}

inode_t *get_inode(int num){
  inode_t *InodeTable = (inode_t *)blocks_get_block(1);
  return &InodeTable[num];
}

int alloc_inode() {
  void *bitmap = get_inode_bitmap();
  for (int i = 0; i < 128; i++){
    if (!bitmap_get(bitmap, i)) {
      bitmap_put(bitmap, i, 1);
      inode_t *node = get_inode(i);
      memset(node, 0, sizeof(inode_t));
      node->block = -1;
      return i;
    }
  }
  return -1;
}

void free_inode (int num) {
  inode_t *node = get_inode(num);
  if (node->block != -1) {
    free_block(node->block);
  }
  memset(node, 0, sizeof(inode_t));
  bitmap_put(get_inode_bitmap(), num, 0);
}

