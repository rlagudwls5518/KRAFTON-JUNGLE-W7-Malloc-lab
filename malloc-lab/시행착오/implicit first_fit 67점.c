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
    "Frog",
    /* First member's full name */
    "김형진",
    /* First member's email address */
    "rlagudwls5518@naver.com",
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

// 기본 매크로 상수
#define WSIZE 4                 // 워드, 헤더, 푸터의 크기 (4바이트)
#define DSIZE 8                 // 더블 워드의 크기 (8바이트)
#define CHUNKSIZE (1 << 12)     // 힙 연장 시 이만큼 (4096 바이트)
#define MAX(x, y) ((x) > (y)? (x): (y))   // 두 값 중 더 큰 값


// 크기 비트와 할당 비트 통합 -> 헤더, 푸터 저장용
#define PACK(size, alloc) ((size) | (alloc))    

// p는 (void *)일 수 있으니 (unsigned char *)로 변환. 포인터가 참조하는 워드 리턴.
#define GET(p) (*(unsigned int *)(p))         

// 포인터가 참조하는 워드에 val을 저장.  
#define PUT(p, val) (*(unsigned int *)(p) = (val)) 

// 주소 p가 헤더일 때, 저장된 size를 반환
#define GET_SIZE(p) (GET(p) & ~0x7)     

// 주소 p가 헤더일 때, 저장된 유효 비트를 반환        
#define GET_ALLOC(p) (GET(p) & 0x1)   

// 블록 주소로, 헤더의 주소 계산 (헤더의 크기만큼 빼줌)
#define HDRP(bp) ((char *)(bp) - WSIZE)             

// 블록 주소로, 푸터의 주소 계산 (푸터 및 다음 헤더의 크기만큼 빼줌)
#define FTRP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)          

// 다음 블록 주소 계산.
#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE(((char *)(bp) - WSIZE)))    

// 이전 블록 주소 계산.
#define PREV_BLKP(bp) ((char *)(bp) - GET_SIZE(((char *)(bp) - DSIZE))) 

static char *heap_listp; // 첫 블록을 가리킴

/*
 * mm_init 할당기 초기화 하는 함수 성공하면 0 아니면 -1 리턴
 */
int mm_init(void)
{
    //(1) 메모리 시스템에서 4워드(16바이트)를 가져와서, 빈 가용리스트를 만듬

    heap_listp = (char *)mem_sbrk(4 * WSIZE);  // 할당된 메모리의 시작주소

    if(heap_listp == (void *)-1) return -1;

    //(2) 16바이트를 채워야 함
    PUT(heap_listp, 0); // 맨 처음 미 사용 블록 4바이트
    PUT(heap_listp + (1 * WSIZE), PACK(DSIZE,1)); // 프롤로그 헤더 (헤더 4바이트+푸터 4바이트)8바이트
    PUT(heap_listp + (2 * WSIZE), PACK(DSIZE,1)); // 프롤로그 푸터 (헤더 4바이트+푸터 4바이트)8바이트
    PUT(heap_listp + (3 * WSIZE), PACK(DSIZE,1)); // 에필로그 헤더(헤더 4바이트 마지막블록이니 표시만 0/1)

    //(3) 가용블록 생성
    heap_listp += (2 * WSIZE);// 프롤로그 푸터다음 새로운 블록 시작위치

    //(4) 힙 확장하기(CHUNKSIZE : 4096바이트로 인식)
    if(extend_heap(CHUNKSIZE/WSIZE) == NULL)return -1;
    
    return 0;
}

// 힙을 초기화 했을때
// 메모리 할당 요청을 받았는데 빈 블록을 못 찾았을때
// 힙 메모리 추가 요청하는 함수
void *extend_heap(size_t words){  // 워드를 매개변수로 받음
    char *ptr;
    size_t size = ALIGN(words * WSIZE); // 그래서 바이트 * 워드를 하는겨

    ptr = mem_sbrk(size);
    if(ptr == (void *) - 1) return NULL;


    // 이전 에필로그 블록에 새로운 헤더랑 푸터가 옴
    PUT(HDRP(ptr), PACK(size,0));
    PUT(FTRP(ptr), PACK(size,0));

    // 에필로그 블록은 추가된 블록 맨뒤로 이동
    PUT(HDRP(NEXT_BLKP(ptr)), PACK(0,1)); 

    return coalesce(ptr);

}

/*
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
//할당하고 시작주소 반환
void *mm_malloc(size_t size)
{
    size_t asize; //실제로 할당되는 블록의 크기
    size_t extend_size;//맞는 사이즈가 없으면 변수만큼 사이즈 추가요청
    char* ptr;

    if(size ==0) return NULL;

    asize = ALIGN(size + DSIZE);//8바이트 배수로 정렬(패딩)

    ptr = find_fit(asize); 

    if(ptr != NULL){
        place(ptr, asize);
        return ptr;
    }
    //맞는 칸이 없으면 힙 연장
    extend_size = MAX(asize, CHUNKSIZE); 
    ptr = extend_heap(extend_size / WSIZE);

    if (ptr == NULL){
        return NULL;
    }
    place(ptr, asize);
    return ptr;
}

/*
 * mm_free - ptr이 가리키는 할당된 메모리 블록 해제
 */

void mm_free(void *ptr)
{
    size_t size = GET_SIZE(HDRP(ptr));//블록의 사이즈를 알기 위해 헤더에서 가져옴

    PUT(HDRP(ptr), PACK(size, 0)); //할당비트를 1에서 0으로만 바꿔주면 해제
    PUT(FTRP(ptr), PACK(size, 0));

    //할당 해제하면 포인터는 그대로인데 포인터도 NULL로 해제를 하나?
    coalesce(ptr);
}

// ptr이 가리키는 블록과 인접 블록을 연결 후, 연결된 블록의 주소 반환
void *coalesce(void *ptr){
    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(ptr))); // 이전 블록의 할당 비트 반환
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(ptr))); // 다음 블록의 할당 비트 반환
    size_t size = GET_SIZE(HDRP(ptr));


    if(prev_alloc && next_alloc){ // 둘다 가득 차있는경우 넘기기 case 1
        return ptr;
    }

    else if(prev_alloc){ // 다음 블록이 비어있는경우 case 2
        size += GET_SIZE(HDRP(NEXT_BLKP(ptr)));
        PUT(HDRP(ptr), PACK(size,0)); // 현재 헤더 수정
        PUT(FTRP(ptr), PACK(size,0)); // 현재 푸터 수정

    }

    else if(next_alloc){ // 이전 블록이 비어있는경우 case 3
        size += GET_SIZE(HDRP(PREV_BLKP(ptr)));
        PUT(HDRP(PREV_BLKP(ptr)), PACK(size,0)); // 현재 헤더 수정
        PUT(FTRP(ptr), PACK(size,0)); // 현재 푸터 수정
        ptr = PREV_BLKP(ptr);// 주소 갱신

    }

    else{ // 앞뒤 둘다 비어있는경우 case 4
        size += GET_SIZE(HDRP(PREV_BLKP(ptr))) + GET_SIZE(HDRP(NEXT_BLKP(ptr)));
        
        PUT(HDRP(PREV_BLKP(ptr)), PACK(size,0)); // 현재 헤더 수정
        PUT(FTRP(NEXT_BLKP(ptr)), PACK(size,0)); // 현재 푸터 수정
        ptr = PREV_BLKP(ptr);// 주소 갱신

    }
    return ptr;

}

void *find_fit(size_t asize){ // first_fit으로 구현
    void * ptr; // 현재검색중인 블록
    size_t block_size; // 현재블록의 크기
    
    // 에필로그 블록 도달하면 종료
    for(ptr = heap_listp; (block_size = GET_SIZE(HDRP(ptr))) != 0; ptr = NEXT_BLKP(ptr)){
        //사이즈에 맞는 가용블록이 있고 할당비트가 0이면 그때 블록 주소 리턴
        if(asize <= block_size &&(GET_ALLOC(HDRP(ptr)) ==0)) return ptr;
    }
    return NULL;

}
//
void place(void *ptr, size_t newsize){
    size_t curr_size, rest_size;
    
    
    curr_size = GET_SIZE(HDRP(ptr)); // 할당할 메모리 크기
    rest_size = curr_size - newsize; // 블록크기 - 할당할 메모리 크기 = 나머지 크기

    if(rest_size >= 4 * WSIZE){// 최소 블록크기 16바이트부터 자를수 있음

        //앞 블록의 헤더
        PUT(HDRP(ptr), PACK(newsize, 1));
        // 앞 블록의 푸터
        PUT(FTRP(ptr), PACK(newsize, 1));

        // 뒷 헤더 만들기
        PUT(HDRP(NEXT_BLKP(ptr)), PACK(rest_size, 0));
        // 뒷 블록의 푸터 만들기
        PUT(FTRP(NEXT_BLKP(ptr)), PACK(rest_size, 0));
    }
    else{
        PUT(HDRP(ptr), PACK(curr_size, 1));
        PUT(FTRP(ptr), PACK(curr_size, 1));
    }

}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */

 //ptr이 가리키는 블록에 할당된 메모리의 크기를 size로 변경
void *mm_realloc(void *ptr, size_t size)
{
   if(ptr == NULL){
        return mm_malloc(size);
   }
   else if(size == 0){
        mm_free(ptr);
        return NULL;
   }

   void *old_ptr = ptr;
   void *new_ptr = mm_malloc(size);

   size_t copy_size = GET_SIZE(HDRP(ptr));

   if(size < copy_size) copy_size = size;


   memcpy(new_ptr, old_ptr, copy_size); // 이전 블록의 내용을 새블록으로 복사
   mm_free(old_ptr); // 이전블록에 할당된 메모리는 free

   return new_ptr;
}