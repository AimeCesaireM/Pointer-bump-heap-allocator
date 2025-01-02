# Pointer-Bumping Allocator

## Introduction
The Pointer-Bumping Allocator is a simple heap allocator that uses pointer bumping for memory allocation. Pointer bumping is a technique where memory is allocated by incrementing a pointer, ensuring constant-time allocation. However, this allocator does not reuse freed blocks, making it straightforward but limited in functionality.

This project demonstrates how a custom memory allocator can be implemented in C using the `mmap` system call to manage virtual address space.

---

## Features

### Implemented Functions

- **`malloc(size_t size)`**: Allocates a block of memory of the specified size, aligning the block to 16-byte boundaries for efficiency.
- **`free(void* ptr)`**: Currently a no-op. Freed memory is not reused.
- **`calloc(size_t nmemb, size_t size)`**: Allocates and zeroes a block of memory for an array of `nmemb` elements of `size` bytes each.
- **`realloc(void* ptr, size_t size)`**: Resizes an existing block, allocating a new block if the requested size is larger than the original, and copying the data.

### Design Highlights

- **Heap Initialization**: Uses `mmap` to allocate a 2GB virtual address space for the heap.
- **Header Metadata**: Each block has a header (`header_s`) storing the size of the block.
- **16-Byte Alignment**: Ensures memory alignment for performance and compatibility.
- **Simple Allocation**: Expands memory linearly without maintaining a free list.

---

## Build Instructions

### Prerequisites

- A C compiler (e.g., `gcc`)
- Linux system (for `mmap` and `sysconf` usage). Use Windows Subsystem for Linux if on a Windows system
- Download and extract the zip of this repo.

### Compilation

To build the allocator as a standalone program with the testing main function:

```bash
gcc -DALLOC_MAIN -o pb_alloc pb-alloc.c
```
To build the entire project, run the Makefile script.
```bash
make
```
### Running Tests

Run the compiled program:

```bash
./pb_alloc
```

---

## Usage Examples

### Example 1: Basic Allocation

```c
void* ptr = malloc(64); // Allocates 64 bytes
```

### Example 2: Array Allocation with `calloc`

```c
void* array = calloc(10, sizeof(int)); // Allocates zeroed memory for 10 integers
```

### Example 3: Resizing Memory with `realloc`

```c
void* ptr = malloc(32);
ptr = realloc(ptr, 64); // Resizes the block to 64 bytes
```

---

## Limitations

1. **No Memory Reuse**: Freed blocks are not reused, potentially leading to inefficient memory usage.
2. **Heap Size Limit**: The allocator is restricted to the predefined heap size (2GB).
3. **No Multithreading Support**: The allocator does not handle concurrency.

---

## Future Work

1. **Free List Implementation**: Add a free list to track and reuse freed blocks, improving memory efficiency.
2. **Thread Safety**: Introduce synchronization mechanisms (e.g., mutexes) to make the allocator thread-safe.
3. **Error Handling**: Enhance error reporting for edge cases.
4. **Heap Shrinking**: Implement functionality to release unused memory back to the operating system.

---

## References

- **POSIX `mmap`**: [man page](https://man7.org/linux/man-pages/man2/mmap.2.html)
- **Memory Alignment**: Explanation of why 16-byte alignment matters.
- **Pointer Bumping**: Overview of the technique and its trade-offs.

---

## Author
This implementation demonstrates fundamental principles of custom memory allocation in C. Contributions and suggestions are welcome!


