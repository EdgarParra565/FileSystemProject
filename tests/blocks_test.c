/**
 * @file blocks_test.c
 * @author Edgar Parra and Lucas Fuji
 *
 * Test driver for the blocks module.
 * Initializes a block device, allocates a block, writes data into it, and
 * prints the block bitmap to verify the allocator is working as expected.
 */
#include <stdio.h>

#include "bitmap.h"
#include "blocks.h"

#define TEST_NAME "block_test.img"

int main(int argc, char **argv) {
  blocks_init(TEST_NAME);

  printf("Block bitmap at the beginning:\n");
  bitmap_print(get_blocks_bitmap(), BLOCK_COUNT);

  int block_num = alloc_block();

  printf("Allocated block no. %d\n", block_num);

  printf("Block bitmap after allocating:\n");
  bitmap_print(get_blocks_bitmap(), BLOCK_COUNT);

  long *block = blocks_get_block(block_num);

  for (int i = 0; i < 42; i++) {
    block[i] = i + 1;
  }

  printf("Written to block %d:", block_num);
  for (int i = 0; i < 42; i++) {
    printf(" %ld", block[i]);
  }
  putchar('\n');

  blocks_free();

  return 0;
}
