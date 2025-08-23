/*
 * memlib.c - a module that simulates the memory system.  Needed because it 
 *            allows us to interleave calls from the student's malloc package 
 *            with the system's malloc package in libc.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>
#include <errno.h>

#include "memlib.h"
#include "config.h"

/* private variables */
static char *mem_start_brk;  /* points to first byte of heap, 힙의 첫 바이트를 가리키는 변수 */
static char *mem_brk;        /* points to last byte of heap, 힙의 마지막 바이트를 가리키는 변수. 여기에다가 sbrk 함수로 새로운 사이지의 힙을 할당받는다.  */
static char *mem_max_addr;   /* largest legal heap address, 힙의 최대 크기(20MB)의 주소를 가리키는 변수 */ 

/* 
 * mem_init - initialize the memory system model
     최대 힙 메모리 공간을 할당받고 초기화해준다. 
 */
void mem_init(void)
{
    /* allocate the storage we will use to model the available VM */
    // config.h에 정의되어 있음, #define MAX_HEAP (20*(1<<20)) : 20971520bytes == 20 MB
    // 먼저 20MB만큼의 MAX_HEAP을 malloc으로 동적할당해온다. 만약 메모리를 불러오는데 실패했다면 에러 메세지를 띄우고 프로그램을 종료한다.
    // 그 시작점을 mem_start_brk라 한다.
    // 아직 힙이 비어 있으므로 mem_brk도 mem_start_brk와 같다.

    if ((mem_start_brk = (char *)malloc(MAX_HEAP)) == NULL) {
	fprintf(stderr, "mem_init_vm: malloc error\n");
	exit(1);
    }

    mem_max_addr = mem_start_brk + MAX_HEAP;  /* max legal heap address */
    mem_brk = mem_start_brk;                  /* heap is empty initially */
}

/* 
 * mem_deinit - free the storage used by the memory system model
 */
// init으로 초기 할당했던 힙 전체 다시 반납
void mem_deinit(void)
{
    free(mem_start_brk);
}

/*
 * mem_reset_brk - reset the simulated brk pointer to make an empty heap
 */
// 시뮬레이션 된 힙을 완전히 비어있는 상태로 되돌림 포인터 초기화
void mem_reset_brk()
{
    mem_brk = mem_start_brk;
}

/* 
 * mem_sbrk - simple model of the sbrk function. Extends the heap 
 *    by incr bytes and returns the start address of the new area. In
 *    this model, the heap cannot be shrunk.
 */

 // byte 단위로 필요 메모리 크기를 입력받아 그 크기만큼 힙을 늘려주고, 새 메모리의 시작 지점을 리턴한다.
 // 힙을 줄이려 하거나 최대 힙 크기를 넘어버리면 리턴한다.
 // old_brk의 주소를 리턴하는 이유: 새로 늘어난 힙의 첫번째 주소이기 때문
// 리턴 할때 왜 형변환을 하는지?? -> 그냥 정수면 정수 캐릭터면 캐릭터를 쓰며냐 되는디
// --> 함수가 매개변수로 받아들여야  하는 포인터가 어떤 자료형에 대한 포인터인지 알 수가 없을수 있기 때문에 
// (void는 모든 종류의 자료형을 받을 수 있다.)

void *mem_sbrk(int incr) 
{
    char *old_brk = mem_brk;

    if ( (incr < 0) || ((mem_brk + incr) > mem_max_addr)) {
        errno = ENOMEM;
        fprintf(stderr, "ERROR: mem_sbrk failed. Ran out of memory...\n");
        return (void *)-1;
    }

    mem_brk += incr;
    return (void *)old_brk;
}

/*
 * mem_heap_lo - return address of the first heap byte
 */
//힙의 첫번째 주소 반환
void *mem_heap_lo()
{
    return (void *)mem_start_brk;
}

/* 
 * mem_heap_hi - return address of last heap byte
 */
//힙의 마지막 주소 반환
void *mem_heap_hi()
{
    return (void *)(mem_brk - 1);
}

/*
 * mem_heapsize() - returns the heap size in bytes
 */
//힙의 사이즈를 바이트로 반환
size_t mem_heapsize() 
{
    return (size_t)(mem_brk - mem_start_brk);
}

/*
 * mem_pagesize() - returns the page size of the system
 */
//시스템의 페이지 사이즈를 반환
size_t mem_pagesize()
{
    return (size_t)getpagesize();
}
