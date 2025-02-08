/*
 * In this file, we implement malloc in terms of Explicit Free List.
 * Originally I plan to code the version of Seglist, but due to the time limit,
 * I decide to make some functions "degenerate".
 * However, the whole file is still written in the style of Seglist
 * and one can actually implement Seglist by extending some of the functions below.
 *
 * Each node in the list consists of
 * top: record the size of the block, the last bit is used to determine if it is free
 * next: store the starting address of the next node in the list, null if there is no next node
 * prev: store the starting address of the previous node in the list, null if it is the first node
 * bottom: record the size of the block, the last bit is used to determine if it is free
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

team_t team = {
    /* Team name */
    "Self-Study",
    /* First member's full name */
    "Bionas",
    /* First member's email address */
    "mathliqh@gmail.com",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""
};

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)


#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))
// find the starting address of the next node in the list
#define NEXT_NODE_ADDRESS(p) ((size_t *)*(size_t *)(((char *) p) + SIZE_T_SIZE))

// find the starting address of the previous node in the list
#define PREV_NODE_ADDRESS(p) ((size_t *)*(size_t *)(((char *) p) + 2 * SIZE_T_SIZE))

// find the place where the starting address of the next node is stored
#define NEXT_NODE_OH(p) (*(size_t **)(((char *) p) + SIZE_T_SIZE))

// find the place where the starting address of the previous node is stored
#define PREV_NODE_OH(p) (*(size_t **)(((char *) p) + 2 * SIZE_T_SIZE))

// find the head of the list of index i (the free list whose nodes are of size in the i-th range)
#define GET_HEAD(i) ((size_t *)*(size_t *)(mem_heap_lo() + i * SIZE_T_SIZE))

// number of overhead blocks for free blocks
#define FREE_OVERHEAD_BLOCKS 4

// number of overhead blocks for allocated blocks
#define ALLOCATED_OVERHEAD_BLOCKS 2

// number of free lists, we are considering the degenerated case (a single explicit free list)
#define SEGLIST_BLOCKS 1

// when nonzero, this records the size of the largest block in the current free list
// and this is used to avoid repeated computations in some test cases
static size_t curr_max;


/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    curr_max = 0;
    // initialize the heads for each free list
    // one more block is used to separate heads and the actual lists
    void *p = mem_sbrk((SEGLIST_BLOCKS + 1) * SIZE_T_SIZE);
    if (p == (void *) -1) {
        printf("Unable to initialize\n");
        return -1;
    }
    // make them zero to start with empty lists
    memset(p, 0, (SEGLIST_BLOCKS + 1) * SIZE_T_SIZE);
    return 0;
}


// check if a block is free by checking the last bit in the top and tail overhead blocks
static int is_free(size_t *p) {
    if (p == NULL)
        return 0;
    return (*p) & 1 ;
}

// print all elements in the heap
// each line represents either a free block or an allocated block
void display_list() {
    char *p = ((char *)mem_heap_lo()) + (SEGLIST_BLOCKS + 1) * SIZE_T_SIZE;
    printf("Start of list \n");
    while (p != mem_heap_hi() + 1) {
        if (is_free((size_t *)p)) {
            printf("A free block of size %d \n", (*(size_t *)p) - 1);
            p += (*(size_t *)p) - 1;
        } else {
            printf("An allocated block of size %d \n", (*(size_t *)p));
            p += (*(size_t *)p);
        }
    }
    printf("End of list \n");
}

// find the corresponding list for the size
// one can implement Seglist by extending this function according to its range of sizes
static size_t find_index(size_t size) {

    return 0;
}

// remove a node from the free list
// information in the overhead blocks of the current node is not changed
static void remove_from_list(size_t *p) {

    size_t *prev = PREV_NODE_ADDRESS(p);
    size_t *next = NEXT_NODE_ADDRESS(p);
    // change the overehads of the next and previous nodes
    if (prev != NULL) {
        NEXT_NODE_OH(prev) = next;
    } else {
        size_t **head = (size_t **)(mem_heap_lo() + find_index(*p - 1) * SIZE_T_SIZE);
        *head = next;
    }
    if (next != NULL)
        PREV_NODE_OH(next) = prev;

    return;
}

// add a free block to list and in the meantime change the information in the overhead blocks
static void add_to_list(size_t *p, size_t size) {
    // update overheads of the current node
    *p = size + 1;
    *(size_t *)(((char *) p) + size - SIZE_T_SIZE) = size + 1;

    size_t i = find_index(size);
    size_t **head =  (size_t **)(mem_heap_lo() + i * SIZE_T_SIZE);
    PREV_NODE_OH(p) = NULL;
    NEXT_NODE_OH(p) = NULL;
    // update overheads of the next and the previous nodes
    if (*head != NULL) {
        NEXT_NODE_OH(p) = *head;
        PREV_NODE_OH(*head) = p;
    }
    *head = p;
}

// for a given size, find the best node in the i-th free list
// which is able to store the information
static size_t *search(size_t i, size_t size) {
    // if this size is greater than the current max size, simply return NULL
    if (curr_max != 0 && curr_max < size)
        return NULL;
    size_t *p = GET_HEAD(i);
    if (p == NULL) {
        return NULL;
    }
    // travel through the i-th list to find the first fit
    size_t *best = NULL;
    while (p != NULL) {
        if (*p - 1 >= size) {
            best = p;
            break;
        }
        if (*p - 1 > curr_max)
            curr_max = *p - 1;
        p = NEXT_NODE_ADDRESS(p);
    }
    // no suitable node
    if (best == NULL) {
        return NULL;
    }
    // find the best node
    while (p != NULL) {
        //printf("%d ", *p);
        if (*p - 1 >= size && *p < *best) {
            best = p;
        }
        if (*p - 1 > curr_max)
            curr_max = *p - 1;
        p = NEXT_NODE_ADDRESS(p);
    }
    if (curr_max == *best - 1)
        curr_max = 0;

    return best;
}

// split the current node into two parts where the first part is the desired size
// and then remove the first part from the list
// the splitting will not happen if the remaining size is too small to store overheads
static void split(size_t *best, size_t size) {
    // remove the whole node
    remove_from_list(best);
    size_t remain_size = *best - 1 - size;
    // if the remaining size is large enough, add back the remaining part
    if (remain_size >= FREE_OVERHEAD_BLOCKS * SIZE_T_SIZE) {

        add_to_list((size_t *)(((char *)best) + size), remain_size);
        *best = size;
        *(size_t *)(((char *) best) + size - SIZE_T_SIZE) = size;
    } else {
        (*best)--;
        (*(size_t *)(((char *) best) + (*best) - SIZE_T_SIZE))--;
    }
}

// find and split a required block in the i-th list
static void *find_best_fit(size_t i , size_t size) {

    size_t * best = search(i, size);

    if (best == NULL)
        return NULL;

    split(best, size);

    return (void *)((char *)best + SIZE_T_SIZE);
}


/* 
 * mm_malloc - Allocate a block firstly by finding a best fit in the existing list
 * if there is no such a fit, then increment brk to get extra space
 */

void *mm_malloc(size_t size)
{
    // calculate the aligned size
    size_t newsize = ALIGN(size) + ALLOCATED_OVERHEAD_BLOCKS * SIZE_T_SIZE;
    size_t i = find_index(newsize);
    void *p;
    // try finding a fit in the list
    while (i < SEGLIST_BLOCKS) {
        if ((p = find_best_fit(i, newsize)) != NULL) {
            return p;
        }
        i++;
    }
    // if there is no such a fit, get some extra space
    p = mem_sbrk(newsize);

    if (p == (void *)-1)
	    return NULL;
    else {
        *(size_t *)p = newsize;
        *(size_t *)(((char *) p) + newsize - SIZE_T_SIZE) = newsize;
        return (void *)((char *)p + SIZE_T_SIZE);
    }
}


/*
 * mm_free - Freeing a block and check if the two blocks next to it are free or not
 * if so, combine these free blocks together and add the result to the list
 */
void mm_free(void *ptr)
{

    size_t *p = (size_t *)(ptr - SIZE_T_SIZE);
    if (p == NULL) {
        printf("NULL Address\n");
        return;
    }
    if (is_free(p)) {
        printf("This block is already free\n");
        return;
    }
    size_t total_size = *p;
    size_t *next_nbhd = (size_t *)(((char *)p) + *p);
    // check if the next block is free
    if ((void *)next_nbhd - 1 != mem_heap_hi()) {
        if (is_free(next_nbhd)) {
            total_size += (*(next_nbhd) - 1);
            remove_from_list(next_nbhd);
        }
    }


    // check if the previous block is free
    size_t *prev_nbhd = (size_t *)(((char *)p) - SIZE_T_SIZE);
    if (is_free(prev_nbhd)) {
        total_size += (*(prev_nbhd) - 1);
        p = (size_t *)(((char *)p) - ((*prev_nbhd) - 1));
        remove_from_list(p);
    }

    // add the combined block to the list
    add_to_list(p, total_size);
    if (total_size > curr_max && curr_max != 0)
        curr_max = total_size;

}

/*
 * mm_realloc - Free the block and allocate new space with some extra optimizations
 */

void *mm_realloc(void *ptr, size_t size)
{
    // deal wtih some special cases
    if (ptr == NULL)
       return mm_malloc(size);
    if (size == 0) {
        mm_free(ptr);
        return NULL;
    }

    // calculate the size of the orginal block and the new aligned size
    size_t prev_size =  *(size_t *)(((char *)ptr) - SIZE_T_SIZE);
    size_t newsize = ALIGN(size) + ALLOCATED_OVERHEAD_BLOCKS * SIZE_T_SIZE;

    // record the information contained in the "next" "prev" blocks
    // this is necessary because our implementation of free() will change these two blocks
    long long temp1 =  *(long long *)ptr;
    long long temp2 = *(((long long *)ptr) + 1);

    // deal with the special case when the block is at the end of the heap
    // this optimization is for some test cases
    if (((char *)ptr) + prev_size - SIZE_T_SIZE == ((char *) mem_heap_hi()) + 1) {
        // when the size is enough, we do not change anything
        if (newsize <= prev_size)
            return ptr;
        // when the size is not enough, add more size directly to the bottom of the heap
        mem_sbrk(newsize - prev_size);
        *(size_t *)(((char *)ptr) - SIZE_T_SIZE) = newsize;
        *(size_t *)(((char *)ptr) - 2 * SIZE_T_SIZE + newsize) = newsize;
        return ptr;
    }

    // when the ptr is not at the end of the heap
    // simply free the block and allocate the space
    // and then copy the original data
    mm_free(ptr);

    size_t i = find_index(newsize);

    void *p = NULL;

    while (i < SEGLIST_BLOCKS) {
        if ((p = search(i, newsize)) != NULL) {
            break;
        }
        i++;
    }

    if (p != NULL) {

        memmove(((char *)p) + 3 * SIZE_T_SIZE, ((char *)ptr) + 2 * SIZE_T_SIZE, prev_size > newsize ? newsize - 4 * SIZE_T_SIZE: prev_size - 4 * SIZE_T_SIZE);


        split(p, newsize);

        p = (void *)(((char *)p) + SIZE_T_SIZE);
        *(long long *)p = temp1;
        *(((long long *)p) + 1) = temp2;

        return p;
    } else {
        p = mem_sbrk(newsize);

        if (p == (void *)-1)
            return NULL;
        else {
            memmove(((char *)p) + 3 * SIZE_T_SIZE, ((char *)ptr) + 2 * SIZE_T_SIZE, prev_size > newsize ? newsize - 4 * SIZE_T_SIZE: prev_size - 4 * SIZE_T_SIZE);
            *(size_t *)p = newsize;
            *(size_t *)(((char *) p) + newsize - SIZE_T_SIZE) = newsize;
            p = (void *)(((char *)p) + SIZE_T_SIZE);
            *(long long *)p = temp1;
            *(((long long *)p) + 1) = temp2;

            return p;
        }
    }
}















