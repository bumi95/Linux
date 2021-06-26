/*
*
*   DKU Operating System Lab
*           Lab2 (Vehicle Production Problem)
*           Student id : 32141925
*           Student name : Park Jong Bum
*
*   lab1_sync_types.h :
*       - lab2 header file.
*       - must contains Vehicle Production Problem's declations.
*
*/

#ifndef _LAB2_HEADER_H
#define _LAB2_HEADER_H

#include <stdio.h>
#include <fcntl.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <assert.h>
#include <pthread.h>
#include <asm/unistd.h>

#define MAX_SIZE 100

/*
 * You need to Declare functions in  here
 */

typedef struct Node { // 차량 노드 구조체
	int car_num; // 차량 번호(0 ~ 4)
	int start_t, prod_num; // 생산 시작 시간과 차량 생산 수
	struct Node *next; // 다음 노드를 가리킬 포인터
}Node;

typedef struct ready_queue { // 레디 큐 구조체
	Node *front, *rear; // 원형 큐로 구현하기 위한 포인터 두 개
}RQ;

typedef struct car_queue { // 차량 생산 큐 구조체
	int balance; // 차량 생산 큐에 들어있는 차량의 수
	Node *front, *rear; // 원형 큐로 구현하기 위한 포인터 두 개
}CQ;

void RQ_init(RQ *r_queue);
void RQ_insert(RQ *r_queue, Node node);
Node RQ_delete(RQ *r_queue);
int RQ_is_empty(RQ *r_queue);

void CQ_init(CQ *c_queue);
void CQ_insert(CQ *c_queue, Node node);
void CQ_delete(CQ *c_queue, int n);

void *producer(void *arg);
void *consumer(void *arg);

#endif /* LAB2_HEADER_H*/
