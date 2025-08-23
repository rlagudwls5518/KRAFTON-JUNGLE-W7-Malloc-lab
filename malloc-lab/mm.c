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
//size보다 큰 가장 가까운 ALIGNMENT의 배수로 만들어준다 => 정렬
#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ~0x7) 

//메모리 할당 시 기본적으로 header와 footer를 위해 필요한 더블워드만큼의 메모리 크기
//size_t: 해당 시스템에서 어떤 객체나 값이 포함할 수 있는 최대 크기의 데이터 
//32비트 시스템이면 4바이트
//64바이트면 8바이트
#define SIZE_T_SIZE (ALIGN(sizeof(size_t))) //

//가용 리스트 메크로 정리
#define WSIZE 4
#define DSIZE 8
#define CHUNKSIZE (1<<12)

#define MAX(x,y) ((X) > (y) ? (x) : (y))
#define PACK(size, alloc) ( (size) | (alloc) )

#define GET(p) (*(unsigned int *)(p))
#define PUT(p,val) (*(unsigned int *)(p)) = val

#define GET_SIZE(p)  (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)

#define HDRP(bp) ((char *)(bp) - WSIZE)
#define FDRP(bp) ((char *)(bp) - GET_SIZE(HDRP(bp)) - DSIZE)

#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE((char *)(bp) - WSIZE))
#define PREV_BLKP(bp) ((char *)(bp) - GET_SIZE((char *)(bp) - DSIZE))

static char *heap_listp;

/*
 * mm_init  할당기 초기화 하는 함수 성곡하면 0 아니면 -1리턴
 */
int mm_init(void)
{
    //(1) 메모리 시스템에서 4워드(16바이트)를 가져와서, 빈 가용리스트를 만듬

    heap_listp = (char *)mem_sbrk(4 * WSIZE);  // 할당된 메모리의 시작주소
    if((heap_listp = mem_sbrk(4*WSIZE)) == (void *)-1) return -1;

    //(2) 16바이트를 채워야 함
    PUT(heap_listp, 0); // 
    PUT(heap_listp + (1 * WSIZE), PACK(DSIZE,1)); // 프롤로그 헤더
    PUT(heap_listp + (2 * WSIZE), PACK(DSIZE,1)); // 프롤로그 푸터
    PUT(heap_listp + (3 * WSIZE), PACK(DSIZE,1)); // 에필로그 헤더

    //(3) 가용블록 생성
    heap_listp += (2*WSIZE);// 프롤로그 블록의 헤더 직후 위치

    //(4) 힙 확장하기(CHUNKSIZE : 4096바이트)
    if(extend_heap(CHUNKSIZE/WSIZE) == NULL)return -1;
    
    return 0;
}
// 힙을 초기화 했을때
// 메모리 할당 요청을 받았는데 빈블록을 못 찾았을때
// 힙 메모리 추가요청하는 함수
void *extend_heap(size_t words){ 
    size_t size = ALIGN(words * WSIZE);

}

/*
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    int newsize = ALIGN(size + SIZE_T_SIZE);
    void *p = mem_sbrk(newsize);
    if (p == (void *)-1)
        return NULL;
    else
    {
        *(size_t *)p = size;
        return (void *)((char *)p + SIZE_T_SIZE);
    }
}

/*
 * mm_free - ptr이 가리키는 할당된 메모리 블록 해제
 */
void mm_free(void *ptr)
{
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
    copySize = *(size_t *)((char *)oldptr - SIZE_T_SIZE);
    if (size < copySize)
        copySize = size;
    memcpy(newptr, oldptr, copySize);
    mm_free(oldptr);
    return newptr;
}

static void *extend_heap(size_t words){

}
static void *coalesce(void *bp){

}
static void *find_fit(size_t asize){

}
static void place(void *bp, size_t newsize){

}