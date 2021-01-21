/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 *
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
        /* Team name */
        "1",
        /* First member's full name */
        "김유석",
        /* First member's email address */
        "13chakbo@naver.com",
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
/* Basic constants and macros */
#define WSIZE 4
#define DSIZE 8
#define CHUNKSIZE (1<<12)

#define MAX(x, y) ((x) > (y) ? (x) : (y))

/* Pack a size and allocated bit into a word */
#define PACK(size, alloc) ((size) | (alloc))

/* Read and write a word at address p */
#define GET(p) (*(unsigned int *)(p))
#define PUT(p, val) (*(unsigned int *)(p) = (val))

/* Read the size and allocated fields from address p */
#define GET_SIZE(p) (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)

/* Given block ptr bp, compute address of its header and footer */
#define HDRP(bp) ((char *)(bp) - WSIZE)
#define FTRP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

/* Given block ptr bp, compute address of next and previous blocks */
#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)))
#define PREV_BLKP(bp) ((char *)(bp) - GET_SIZE(((char *)(bp) - DSIZE)))

static void *extend_heap(size_t words);
static void *coalesce(void *bp);
static void *find_fit(size_t asize);
static void place(void* bp, size_t asize);


static char *heap_listp;
/*
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    if ((heap_listp = mem_sbrk(4*WSIZE)) == (void *)-1)
        return -1;
    PUT(heap_listp, 0);
    PUT(heap_listp + (1*WSIZE), PACK(DSIZE, 1));
    PUT(heap_listp + (2*WSIZE), PACK(DSIZE, 1));
    PUT(heap_listp + (3*WSIZE), PACK(0, 1));
    heap_listp += (2*WSIZE);

    if (extend_heap(CHUNKSIZE/WSIZE) == NULL)
        return -1;
    return 0;
}

static void *extend_heap(size_t words)
{
    char *bp;
    size_t size;

    /* Allocate an even number of words to maintain alignments */
    size = (words % 2) ? (words+1) * WSIZE : words * WSIZE;
    if ((bp = mem_sbrk(size)) == (void *)-1)
        return NULL;

    /* Initialize free block header/footer and the epilogue header */
    PUT(HDRP(bp), PACK(size, 0));	/* Free block header */
    PUT(FTRP(bp), PACK(size, 0));	/* Free block footer */
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1)); /* New epilogue header */

    /* Coalesce if the previous block was free */
    return coalesce(bp);
}

/*
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    size_t asize;   /* Adjusted block size */
    size_t extendsize;  /* Amount to extend heap if no fit */
    char *bp;

    /* Ignore spurious requests */
    if (size == 0)
        return NULL;

    /* Adjust block size to include overhead and alignment reqs. */
    if (size <= DSIZE)
        asize = 2*DSIZE; // size + header + footer
    else
        asize = DSIZE * ((size + (DSIZE) + (DSIZE-1)) / DSIZE);
    /* <송예은> 이 if, else문은 size가 더블워드 사이즈보다 작거나 같으면 최소 블록 크기로 맞춰주고, 아니라면 인접 8의 배수로 올려주는 코드인데,
    우리는 매크로에서 ALIGN을 정의했기때문에, asize = MAX(ALIGN(size) + DSIZE, 2*DSIZE) 로 바꿔주면 같은 의미라서
    if, else 문을 한 줄의 코드로 줄일 수 있습니다. */
    

    /* Search the free list for a fit */
    if ((bp = find_fit(asize)) != NULL) {
        place(bp, asize);
        return bp;
    }

    /* No fit found. Get more memory and place the block */
    extendsize = MAX(asize, CHUNKSIZE);
    if ((bp = extend_heap(extendsize/WSIZE)) == NULL)
        return NULL;
    place(bp, asize);
    return bp;
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *bp)
{
    size_t size = GET_SIZE(HDRP(bp));

    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
    coalesce(bp);
}

static void *coalesce(void *bp)
{
    size_t prev_alloc = GET_ALLOC(HDRP(PREV_BLKP(bp)));
    /* <송예은> 이전 블럭의 할당 상태를 구할 때, 헤더로 구하는 것과 푸터로 구하는 것이 값이 똑같아서 상관은 없지만,
    실제로는 이전 블럭의 푸터와 다음 블록의 헤더를 확인하는 것이므로 
    prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp))); 로 쓰는 것이 더 직관적이라 생각합니다.*/

    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));

    if (prev_alloc && next_alloc) {
        return bp; 
        // <송예은> 이 부분도 전혀 상관은 없지만, if문 밖 맨아래 return bp; 를 해주고 있어서 if문 안쪽을 비워두면 중복된 코드를 피할 수 있습니다.
    }

    else if (prev_alloc && !next_alloc) {
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));
    }

    else if (!prev_alloc && next_alloc) {
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        PUT(FTRP(PREV_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
        /* <송예은> 이 부분에서 두번의 PUT과정보다 bp = PREV_BLKP(bp); 를 먼저 해주면 
        PUT(HDRP(bp)), PACK(size, 0));
        PUT(FTRP(bp)), PACK(size, 0));
        위와 같이 PUT을 바꿀 수 있습니다. 
        coalesce 분기 내의 PUT 함수를 통일할 수 있어 코드가 더 깔끔해집니다.
        */
    }
    else {
        size += GET_SIZE(HDRP(PREV_BLKP(bp))) + GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        PUT(FTRP(PREV_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
        /* <송예은> (위와 동일합니다.)
        이 부분에서 두번의 PUT과정보다 bp = PREV_BLKP(bp); 를 먼저 해주면 
        PUT(HDRP(bp)), PACK(size, 0));
        PUT(FTRP(bp)), PACK(size, 0));
        위와 같이 PUT을 바꿀 수 있습니다. 
        coalesce 분기 내의 PUT 함수를 통일할 수 있어 코드가 더 깔끔해집니다.
        */
    }

    return bp;
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
    void *oldptr = ptr;
    void *newptr;
    size_t copySize;

    newptr = mm_malloc(size);
    if (newptr == NULL)
        return NULL;
    copySize = GET_SIZE(HDRP(newptr));
    if (size < copySize)
        copySize = size;
    memcpy(newptr, oldptr, copySize);
    mm_free(oldptr);
    return newptr;
}

static void *find_fit(size_t asize) {
    void *current_block = heap_listp;
    while (GET_SIZE(HDRP(current_block)) != 0) {
        if (!GET_ALLOC(HDRP(current_block))) {
            if (GET_SIZE(HDRP(current_block)) >= asize) {
            /* <송예은> 이 중첩된 If문은 and 로 묶어 아래와 같이 하나의 if문으로 바꿀 수 있습니다.
            if ((!GET_ALLOC(HDRP(current_block))) && (GET_SIZE(HDRP(current_block)) >= asize)) 
            */
                return current_block;
            }
        }
        current_block = NEXT_BLKP(current_block);
    }
    return NULL;
}

static void place(void* bp, size_t asize) {
    size_t size = GET_SIZE(HDRP(bp));
    PUT(HDRP(bp), PACK(asize, 1));
    PUT(FTRP(bp), PACK(asize, 1));
    if ((size - asize) >= DSIZE) { 
        PUT(HDRP(NEXT_BLKP(bp)), PACK((size-asize), 0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK((size-asize), 0));
        /*<송예은> 가용블록이 변했으니 coalesce를 실행해서 연결해주면 더 좋을 것 같습니다.*/
    }
}