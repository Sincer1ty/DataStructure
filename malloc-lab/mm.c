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
	"ateam",
	/* First member's full name */
	"Harry Bovik",
	/* First member's email address */
	"bovik@cs.cmu.edu",
	/* Second member's full name (leave blank if none) */
	"",
	/* Second member's email address (leave blank if none) */
	""};

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ~0x7)

#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

static char *heap_listp;

#define WSIZE 4
#define DSIZE 8
#define CHUNKSIZE (1 << 12)

#define GET(p) (*(unsigned int *)p)
#define PUT(p, val) (*(unsigned int *)(p) = val)

#define PACK(size, alloc) (size | alloc)

#define GET_SIZE(p) (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)

#define HDRP(bp) ((char *)(bp) - WSIZE)
#define FTRP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE(((char *)(bp) - WSIZE)))
#define PREV_BLKP(bp) ((char *)(bp) - GET_SIZE(((char *)(bp) - DSIZE)))

#define MAX(x, y) (x > y ? x : y)

static void *coalesce(void *bp)
{

	size_t prev_alloc = GET_ALLOC(HDRP(PREV_BLKP(bp))); // 이전 블록의 할당 여부
	size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
	size_t size = GET_SIZE(HDRP(bp));

	// 이전 블록과 다음 블록 모두 할당 상태
	if (prev_alloc && next_alloc)
	{
		return bp;
	}

	// 다음 블록만 가용 상태
	else if (prev_alloc && !next_alloc)
	{
		size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
		PUT(HDRP(bp), PACK(size, 0));
		PUT(FTRP(bp), PACK(size, 0));
	}

	// 이전 블록만 가용 상태
	else if (!prev_alloc && next_alloc)
	{
		size += GET_SIZE(HDRP(PREV_BLKP(bp)));
		PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
		PUT(FTRP(bp), PACK(size, 0));
		bp = PREV_BLKP(bp);
	}

	// 이전 블록과 다음 블록 모두 가용 상태
	else
	{
		size += GET_SIZE(HDRP(PREV_BLKP(bp))) + GET_SIZE(HDRP(NEXT_BLKP(bp)));
		PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
		PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
		bp = PREV_BLKP(bp);
	}

	return bp;
}

static void *extend_heap(size_t words)
{
	char *bp;
	size_t size;

	// 정렬될 수 있도록 size 조절
	int adjust = ALIGNMENT / WSIZE;
	size = (words % adjust) ? (words + (adjust - (words % adjust))) * WSIZE : words * WSIZE;
	if ((long)(bp = mem_sbrk(size)) == -1)
	{
		return NULL;
	}

	// 가용 블록 헤더/푸터 & 에필로그 헤더 초기화
	PUT(HDRP(bp), PACK(size, 0));
	PUT(FTRP(bp), PACK(size, 0));
	PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1));

	// 이전 블록이 가용 블록이면 합치기
	return coalesce(bp);
}

/*
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
	// 초기 가용 리스트 만들기
	if ((heap_listp = mem_sbrk(4 * WSIZE)) == (void *)-1)
	{
		// 실패하면 return -1
		return -1;
	}
	PUT(heap_listp, 0);
	PUT(heap_listp + 1 * WSIZE, PACK(DSIZE, 1));
	PUT(heap_listp + 2 * WSIZE, PACK(DSIZE, 1));
	PUT(heap_listp + 3 * WSIZE, PACK(0, 1));
	heap_listp += 2 * WSIZE;

	if (extend_heap(1) == NULL)
	{
		return -1;
	}
	if (extend_heap(CHUNKSIZE / WSIZE) == NULL)
	{
		return -1;
	}

	return 0;
}

static void place(void *bp, size_t asize)
{
	size_t originsize = GET_SIZE(HDRP(bp));
	size_t csize = originsize - asize;

	// 옵션으로 초과부분 분할 : 나머지 부분의 크기가 최소 블록 크기와 같거나 큰 경우에만 분할
	if (csize >= (DSIZE + ALIGNMENT))
	{
		// 요청한 블록 배치 : 요청 블록을 가용 블록의 시작 부분에 배치
		PUT(HDRP(bp), PACK(asize, 1));
		PUT(FTRP(bp), PACK(asize, 1));
		bp = NEXT_BLKP(bp);
		PUT(HDRP(bp), PACK(csize, 0));
		PUT(FTRP(bp), PACK(csize, 0));
	}
	else {
		PUT(HDRP(bp), PACK(originsize, 1));
		PUT(FTRP(bp), PACK(originsize, 1));
	}
}

static void *find_fit(size_t asize)
{
	// 묵시적 가용 리스트에서 first fit 검색 수행
	void *bp;

	// 가용리스트를 처음부터 검색
	for (bp = heap_listp; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp))
	{
		// 가용 블록이면서 size가 asize보다 크거나 같은지
		if (!GET_ALLOC(HDRP(bp)) && (asize <= GET_SIZE(HDRP(bp))))
		{
			// 맞는 가용 블록 포인터 반환
			return bp;
		}
	}

	// 못 찾으면 NULL 반환
	return NULL;
}

/*
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
	size_t asize;
	size_t extendsize;
	char *bp;

	if (size == 0)
	{
		return NULL;
	}

	// 헤더 + 푸터 + size 를 정렬한 블록 크기
	if (size <= ALIGNMENT)
	{
		asize = DSIZE + ALIGNMENT;
	}
	else
		asize = ALIGN(size + DSIZE);

	if ((bp = find_fit(asize)) != NULL)
	{
		place(bp, asize);
		// 8바이트로 정렬된 포인터 반환
		return bp;
	}

	extendsize = MAX(asize, CHUNKSIZE);
	if ((bp = extend_heap(extendsize / WSIZE)) == NULL)
	{
		return NULL;
	}
	place(bp, asize);
	return bp;

	// int newsize = ALIGN(size + SIZE_T_SIZE);
	// void *p = mem_sbrk(newsize);
	// if (p == (void *)-1)
	// return NULL;
	// else {
	//     *(size_t *)p = size;
	//     return (void *)((char *)p + SIZE_T_SIZE);
	// }
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
	size_t size = GET_SIZE(HDRP(ptr));

	PUT(HDRP(ptr), PACK(size, 0));
	PUT(FTRP(ptr), PACK(size, 0));
	coalesce(ptr);
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
	copySize = *(size_t *)((char *)oldptr - WSIZE);
	// size : 헤더 + 푸터 제외 페이로드 크기
	// copySize : 헤더 + 푸터 + 페이로드 + 1
	if (size < copySize)
		copySize = size;
	memcpy(newptr, oldptr, copySize);
	mm_free(oldptr);
	return newptr;
}