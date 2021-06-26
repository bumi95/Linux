/*
*	DKU Operating System Lab
*	    Lab1 (Scheduler Algorithm Simulator)
*	    Student id : 32141925
*	    Student name : Park Jong Bum
*
*   lab1_sched_types.h :
*       - lab1 header file.
*       - must contains scueduler algorithm function's declations.
*
*/

#ifndef _LAB1_HEADER_H
#define _LAB1_HEADER_H


/*
 * You need to Declare functions in  here
 */
typedef struct { // 프로세스를 나타낼 구조체
	char p_name; // 프로세스 이름 (A ~ E)
	int cpu_t, arri_t, p_priority; // 수행 시간, 도착 시간, 우선 순위
}my_proc;

typedef struct { // 레디 큐 구조체, 원형 큐로 구현
	my_proc RQ[6]; // 레디 큐의 최대 크기는 6 (프로세스의 개수는 5개이고 full, empty 상태를 나타내야하기 때문)
	my_proc *front;
	my_proc *rear;
}my_queue;

void my_queue_init(my_queue *que);
void my_queue_insert(my_queue *que, my_proc proc);
my_proc my_queue_delete(my_queue *que);
int my_queue_is_empty(my_queue *que);
void SFJ_queue_sort(my_queue *que);
int my_pow(int n, int m);
void My_sort(my_proc *proc, int n);
void My_FCFS(my_proc *proc, int n);
void My_RRq1(my_proc *proc, int n);
void My_RRq4(my_proc *proc, int n);
void My_SJF(my_proc *proc, int n);
void My_MLFQq1(my_proc *proc, int n);
void My_MLFQq2(my_proc *proc, int n);

#endif /* LAB1_HEADER_H*/



