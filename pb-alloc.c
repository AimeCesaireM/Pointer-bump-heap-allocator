// ==============================================================================
/**
// ==============================================================================
/**
 * pb-alloc.c
 *
 * A _pointer-bumping_ heap allocator.  This allocator *does not re-use* freed
 * blocks.  It uses _pointer bumping_ to expand the heap with each allocation.
 **/
// ==============================================================================



// ==============================================================================
// INCLUDES

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>

#include "safeio.h"
// ==============================================================================



// ==============================================================================
// MACRO CONSTANTS AND FUNCTIONS

/** The system's page size. */
#define PAGE_SIZE sysconf(_SC_PAGESIZE)

/**
 * Macros to easily calculate the number of bytes for larger scales (e.g., kilo,
 * mega, gigabytes).
 */
#define KB(size)  ((size_t)size * 1024)
#define MB(size)  (KB(size) * 1024)
#define GB(size)  (MB(size) * 1024)

/** The virtual address space reserved for the heap. */
#define HEAP_SIZE GB(2)
// ==============================================================================

size_t roundUp(size_t size);

// ==============================================================================
// TYPES AND STRUCTURES

/** A header for each block's metadata. */
typedef struct header {

  /** The size of the useful portion of the block, in bytes. */
  size_t size;
  
} header_s;
// ==============================================================================



// ==============================================================================
// GLOBALS

/** The address of the next available byte in the heap region. */
static intptr_t free_addr  = 0;

/** The beginning of the heap. */
static intptr_t start_addr = 0;

/** The end of the heap. */
static intptr_t end_addr   = 0;
// ==============================================================================



// ==============================================================================
/**
 * The initialization method.  If this is the first use of the heap, initialize it.
 */

void init () {

  // Only do anything if there is no heap region (i.e., first time called).
  if (start_addr == 0) {

    DEBUG("Trying to initialize");
    
    // Allocate virtual address space in which the heap will reside. Make it
    // un-shared and not backed by any file (_anonymous_ space).  A failure to
    // map this space is fatal.
    void* heap = mmap(NULL,  HEAP_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS,-1, 0);
    if (heap == MAP_FAILED) {
      ERROR("Could not mmap() heap region");
    }

    // Hold onto the boundaries of the heap as a whole.
    start_addr = (intptr_t)heap;
    end_addr   = start_addr + HEAP_SIZE;
    free_addr  = start_addr;

    // DEBUG: Emit a message to indicate that this allocator is being called.
    DEBUG("bp-alloc initialized");

  }

} // init ()
// ==============================================================================


// ==============================================================================
/**
 * Allocate and return `size` bytes of heap space.  Expand into the heap region
 * via _pointer bumping_.
 *
 * \param size The number of bytes to allocate.

 * \return A pointer to the allocated block, if successful; `NULL` if
 *         unsuccessful.
 */
 
 size_t roundUp(size_t size){
 	size_t newSize = size + 16;
 	size_t mod = newSize % 16;
 	newSize = newSize - mod;
 	return newSize;
 }
 
void* malloc (size_t size) {

  init();			// init() does something if there's no heap region allocated yet

  if (size == 0) {
    return NULL;	// If you ask me for a box but you're not going to keep anything in it, then I'm not giving you anything
  }
  size = roundUp (size);
  size_t padding = 16 - ((sizeof(header_s ))%16);

  size_t    total_size = size + sizeof(header_s) + padding;	//calculate the total size of the block: the size requested (where useful stuff's gonna go) plus the size of the header
  header_s* header_ptr = (header_s*)free_addr;		//create a new header (technically point to a space that will contain a header) at the next available byte in the heap
  void*     block_ptr  = (void*)(free_addr + sizeof(header_s) + padding);	//point to the next byte after the header **MODIFIED

  intptr_t new_free_addr = free_addr + total_size;		// create a local variable that stores the new start of the free space (also technically, the end of the block we just created). (Bump the pointer)
  if (new_free_addr > end_addr) {

    return NULL;/* not likely to happen in real life, but this is in the case we have filled up the whole heap ( = if a new block would overflow)
	then you want to end malloc() and return NULL. No memory will be allocated, and the actual program running this will crash*/

  } else {

    free_addr = new_free_addr; //update the global pointer that's tracking the start of the free space

  }

  header_ptr->size = size; // Assign the 'size' property of the header of our block to be the size requested. of course
  return block_ptr; 
  // Overall a pretty inflexible method that returns a block that fits exactly the size requested.

} // malloc()
// ==============================================================================



// ==============================================================================
/**
 * Deallocate a given block on the heap.  Add the given block (if any) to the
 * free list.
 *
 * \param ptr A pointer to the block to be deallocated.
 */
void free (void* ptr) {

  DEBUG("free(): ", (intptr_t)ptr);

} // free()
// ==============================================================================



// ==============================================================================
/**
 * Allocate a block of `nmemb * size` bytes on the heap, zeroing its contents.
 *
 * \param nmemb The number of elements in the new block.
 * \param size  The size, in bytes, of each of the `nmemb` elements.
 * \return      A pointer to the newly allocated and zeroed block, if successful;
 *              `NULL` if unsuccessful.
 */
void* calloc (size_t nmemb, size_t size) {

  // Allocate a block of the requested size.
  size_t block_size = nmemb * size;
  void*  block_ptr  = malloc(block_size);

  // If the allocation succeeded, clear the entire block.
  if (block_ptr != NULL) {
    memset(block_ptr, 0, block_size);
  }

  return block_ptr;
  
} // calloc ()
// ==============================================================================



// ==============================================================================
/**
 * Update the given block at `ptr` to take on the given `size`.  Here, if `size`
 * fits within the given block, then the block is returned unchanged.  If the
 * `size` is an increase for the block, then a new and larger block is
 * allocated, and the data from the old block is copied, the old block freed,
 * and the new block returned.
 *
 * \param ptr  The block to be assigned a new size.
 * \param size The new size that the block should assume.
 * \return     A pointer to the resultant block, which may be `ptr` itself, or
 *             may be a newly allocated block.
 */
void* realloc (void* ptr, size_t size) {

  if (ptr == NULL) {
    return malloc(size); // if you ask me to reallocate a null block, i'll just give you a new one with an exact size fit
  }

  if (size == 0) { //if you ask me to change the size of a block to zero, then that's just freeing it. Doesn't make sense to return a pointer to that block
  // though I suppose free() would then modify the free_addr so you keep a pointer to the beginning of free space.
    free(ptr);
    return NULL;
  }

  header_s* old_header = (header_s*)((intptr_t)ptr - sizeof(header_s) - (16 - (sizeof(header_s ))%16));// taking into account of the padding set in malloc() 
  size_t    old_size   = old_header->size;
  //get a pointer to the old header and its size.
  
  if (size <= old_size) { //if you want to exchnge a big box for a small box, i'll just let you keep the big box
    return ptr;
  }


  void* new_ptr = malloc(size);// otherwise if you want a bigger box; I'll make you a box that's exactly the size you want
  
  if (new_ptr != NULL) {
    memcpy(new_ptr, ptr, old_size);// Assuming nothing went wrong, I'll copy your stuff from the small box to the bigger box
    free(ptr);// then I'll clean the small box
  }
  return new_ptr;// You get to keep the bigger box, because I'm kind :)
  
} // realloc()
// ==============================================================================



#if defined (ALLOC_MAIN)
// ==============================================================================
/**
 * The entry point if this code is compiled as a standalone program for testing
 * purposes.
 */
int main () {

  // Allocate a few blocks, then free them.
  void* x = malloc(16);
  void* y = malloc(64);
  void* z = malloc(32);

  free(z);
  free(y);
  free(x);

  return 0;
  
} // main()
// ==============================================================================
#endif
