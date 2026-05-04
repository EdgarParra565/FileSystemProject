# NUFS — A Userspace File System in C

NUFS is a small Unix-like file system implemented from scratch in C and exposed
to the kernel through [FUSE](https://github.com/libfuse/libfuse). The whole
file system lives inside a single 1 MB disk-image file, but once mounted it
behaves like any other directory on Linux — you can `ls`, `mkdir`, `cat`,
`echo > file`, `mv`, `rm`, etc., and the changes are persisted to the image.

Built as a course project for CS3650 (Computer Systems) at Northeastern.

**Authors:** Edgar Parra and Lucas Fuji

---

## Features

- Persistent on-disk layout backed by an `mmap`'d image file
- Inode-based file representation (16-byte inodes, refs / mode / size / block / atime / mtime)
- Free-space management with two bitmaps (block bitmap + inode bitmap)
- Hierarchical directories with `name → inode` entries (48-byte names, up to 64 entries per directory block)
- Read, write, create, delete, rename, truncate, and `utimens`
- Nested directories (`mkdir -p`-friendly), `rmdir`, `unlink`
- Permission bits and file-type metadata reported correctly through `stat(2)`

### Limitations (by design — it's a teaching project)

- Single direct block pointer per inode → **max file size is one block (4 KB)**
- Total disk size is 1 MB (256 blocks × 4 KB)
- No symlinks, no hard links beyond the one created at `mknod`, no journaling

---

## Architecture

NUFS is layered. Each layer only talks to the one directly below it:

```
       user / shell  (ls, cat, echo, mkdir, rm, ...)
              │
              ▼
         FUSE kernel module
              │
              ▼
   ┌──────────────────────────┐
   │  nufs.c   (FUSE callbacks: getattr, readdir,
   │            read, write, mknod, mkdir, rename, ...)
   └──────────────┬───────────┘
                  ▼
   ┌──────────────────────────┐
   │  storage.c  (path resolution, file/dir orchestration)
   └──────────────┬───────────┘
                  ▼
   ┌──────────────┬───────────┐
   │  inode.c     │ directory.c
   │  (inode      │ (dirent table inside a directory block)
   │   alloc/free)│
   └──────┬───────┴─────┬─────┘
          ▼             ▼
   ┌──────────────────────────┐
   │  blocks.c + bitmap.c
   │  (mmap'd disk image, block alloc, free-bitmap)
   └──────────────────────────┘
```

### On-disk layout

```
┌─────────────┬──────────────┬─────────────┬────────────────────────────┐
│ block bitmap│ inode bitmap │ inode table │ data blocks ...            │
│   (block 0) │              │             │                            │
└─────────────┴──────────────┴─────────────┴────────────────────────────┘
```

---

## Building

Requires a Linux machine with FUSE 2 development headers and `pkg-config`:

```bash
sudo apt-get install libfuse-dev pkg-config libtest-simple-perl
make
```

This produces the `nufs` binary.

## Mounting

```bash
make mount    # creates ./mnt and mounts a fresh data.nufs there
```

You can now interact with `./mnt` like any directory:

```bash
echo "hello" > mnt/greeting.txt
mkdir mnt/notes
ls -la mnt/
cat mnt/greeting.txt
```

To unmount:

```bash
make unmount
```

## Running the test suite

```bash
make test
```

This runs `test.pl`, a Perl script that exercises the file system through the
mount point (file create / read / write / nested directories / rename /
deletion / etc.).

## Debugging

```bash
make gdb      # launches nufs under gdb attached to ./mnt
```

---

## Project layout

| File / Dir            | Purpose                                                    |
|-----------------------|------------------------------------------------------------|
| `nufs.c`              | FUSE callback layer — translates VFS calls into storage ops |
| `storage.{c,h}`       | Top-level FS API — path lookup, file & directory operations |
| `inode.{c,h}`         | Inode allocation, freeing, and lookup                       |
| `directory.{c,h}`     | Directory entry table (`dirent_t`) management               |
| `blocks.{c,h}`        | `mmap`'d disk-image abstraction and block allocator         |
| `bitmap.{c,h}`        | Free-list bitmap helpers                                    |
| `slist.{c,h}`         | Simple linked list of strings (used for path splitting)     |
| `tests/`              | Unit tests for `bitmap`, `blocks`, and `slist`              |
| `test.pl`             | End-to-end Perl test suite run against the mounted FS       |
| `Makefile`            | Build, mount, unmount, test, and gdb targets                |

`bitmap.{c,h}`, `blocks.{c,h}`, and `slist.{c,h}` are CS3650 starter helpers.
Everything else (the inode layer, directory layer, storage layer, and FUSE
glue) was implemented for this project.
