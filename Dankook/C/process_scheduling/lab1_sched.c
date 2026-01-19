/*
*	DKU Operating System Lab
*	    Lab1 (Scheduler Algorithm Simulator)
*	    Student id : 32141925
*	    Student name : Park Jong Bum
*
*   lab1_sched.c :
*       - Lab1 source file.
*       - Must contains scueduler algorithm function'definition.
*
*/

#include <aio.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <assert.h>
#include <pthread.h>
#include <asm/unistd.h>

#include "lab1_sched_types.h"

/*
 * you need to implement FCFS, RR, SPN, SRT, HRRN, MLFQ scheduler. 
 */
void my_queue_init(my_queue *que) { // 레디 큐 생성자 함수

	que->front = que->RQ;
	que->rear = que->RQ; // 큐를 비어있는 상태로 초기화
}

void my_queue_insert(my_queue *que, my_proc proc) { // 레디 큐 삽입 함수

	if(que->RQ+6 == que->front) { // front의 주소가 RQ[6]이 될 경우 RQ[0]으로 주소 이동
		que->front = que->RQ;
	}
	memcpy(que->front, &proc, sizeof(my_proc)); // 인자로 받아온 프로세스를 레디 큐에 삽입
	que->front++; // front 위치 이동
}

my_proc my_queue_delete(my_queue *que) { // 레디 큐 삭제 함수

	if(que->RQ+6 == que->rear) { // rear의 주소가 RQ[6]이 될 경우 RQ[0]으로 주소 이동
		que->rear = que->RQ;
	}
	my_proc temp; // 삭제할 프로세스를 반환하기 위한 노드
	memcpy(&temp, que->rear, sizeof(my_proc)); // 반환할 노드에 삭제할 프로세스 복사
	que->rear++; // rear 위치 이동
	return temp; // 삭제한 프로세스 반환
}

int my_queue_is_empty(my_queue *que) { // 레디 큐 empty case check

	if(que->front == que->rear) {
		return 1; // 레디 큐가 비어 있다면 1 반환
	}
	else {
		return 0; // 비어있지 않으면 0 반환
	}
}

void My_sort(my_proc *proc, int n) { // 프로세스 구조체 배열을 도착 시간 순으로 정렬하기 위한 함수
	my_proc temp;
	int i, j;
	
	for(i=0; i<n; i++) {
		for(j=i+1; j<n; j++) {
			if(proc[i].arri_t > proc[j].arri_t) { // 선택 정렬
				temp = proc[j];
				proc[j] = proc[i];
				proc[i] = temp;
			}
			else if(proc[i].arri_t == proc[j].arri_t) { // 도착 시간이 같다면 이름 순으로 정렬
				if(proc[i].p_name > proc[j].p_name) {
					temp = proc[j];
					proc[j] = proc[i];
					proc[i] = temp;
				}
			}
		}
	}
}

int my_pow(int n, int m) { // n제곱 함수
	int res = 1;
	while(m>0) {
		res *= n;
		m--;
	}
	return res;
}

void SJF_queue_sort(my_queue *que) { // SJF 스케줄링을 위해 레디 큐에 있는 프로세스 중 수행 시간이 가장 짧은 프로세스를 
				    // 먼저 수행시키기 위해 레디 큐의 맨 앞으로(rear의 위치로) 이동시키는 함수
	my_proc *temp; // 정렬을 위한 임시 프로세스 노드 생성
	if(que->rear+1 == que->RQ+6) { // 임시 프로세스 노드가 rear의 다음 위치에 있는 프로세스 노드를 가리키도록 함
		temp = que->RQ;
	}
	else {
		temp = que->rear+1;
	}
	while(que->front != temp) { // front가 가리키는 프로세스 노드까지 순회하면서 수행 시간이 가장 짧은 프로세스를 찾아 옮긴다
		if(temp == que->RQ+6) {
			temp = que->RQ;
		}
		if(temp->cpu_t < que->rear->cpu_t) {
			my_proc tmp;
			memcpy(&tmp, que->rear, sizeof(my_proc));
			memcpy(que->rear, temp, sizeof(my_proc));
			memcpy(temp, &tmp, sizeof(my_proc));
		}
		temp++;
	}
}

void My_FCFS(my_proc *proc, int n){ // First Come First Served scheduling 함수
	int p_num = 0; // 레디 큐에 삽입되지 않은 프로세스 중에서 도착 시간이 가장 빠른 프로세스의 인덱스
	int count = 0; // 현재 시각을 나타냄(프로세스의 도착 시간과 비교하기 위한 변수)

	My_sort(proc, n); // 도착 시간 순으로 프로세스 정렬

	my_queue que;
	my_queue_init(&que); // 레디 큐의 선언과 초기화

	printf("FCFS : ");
	
	while(1) { // FCFS 스케줄링 시작
		if(p_num<5) { // 오버플로우 방지를 위한 조건
			if(proc[p_num].arri_t == count) { // 해당 프로세스의 도착 시간과 현재 시각이 같다면
				my_queue_insert(&que, proc[p_num]); // 레디 큐에 삽입
				p_num++;
			}
		}
		if(my_queue_is_empty(&que)) {
			break; // 레디 큐가 비어있다면 워크로드를 모두 수행한 것이므로 스케줄링 종료
		}
		my_proc ret_proc = my_queue_delete(&que); // 프로세스 수행을 위해 레디 큐에서 삭제
		while(ret_proc.cpu_t>0) { // 수행 시간만큼 반복
			if(p_num<5) { // 오버플로우 방지를 위한 조건
				if(proc[p_num].arri_t == count) {
					my_queue_insert(&que, proc[p_num]);
					p_num++;
				}
			}
			printf("%c ", ret_proc.p_name); // 스케줄링 결과를 보기위해 수행중인 프로세스 이름 출력
			ret_proc.cpu_t--; // 수행 시간 1 감소
			count++; // 현재 시각 1 증가
		}
	}
	printf("\n");
}

void My_RRq1(my_proc *proc, int n) { // Round Robin scheduling 함수 time quantum = 1
	int p_num = 0;
	int count = 0;

	My_sort(proc, n);
	
	my_queue que;
	my_queue_init(&que);

	printf("RR(q=1) : ");

	while(1) {
		int time_q=0; // time quantum 변수
		if(p_num<5) {
			if(proc[p_num].arri_t == count) {
				my_queue_insert(&que, proc[p_num]);
				p_num++;
			}
		}
		if(my_queue_is_empty(&que)) {
			break;
		}
		my_proc ret_proc = my_queue_delete(&que);
		while(ret_proc.cpu_t>0) {
			if(p_num<5) {
				if(proc[p_num].arri_t == count) {
					my_queue_insert(&que, proc[p_num]);
					p_num++;
				}
			}
			if(my_queue_is_empty(&que)==0 && time_q>=1) {
					// 레디 큐에 프로세스가 존재하고 현재 time quantum이 1 이상이면
					// 현재 프로세스를 레디 큐의 마지막에 다시 추가하고 대기중인 다음 프로세스 수행 시작
				my_queue_insert(&que, ret_proc);
				break;
			}
			printf("%c ", ret_proc.p_name);
			ret_proc.cpu_t--;
			count++;
			time_q++; // time quantum 1 증가
		}
	}
	printf("\n");
}

void My_RRq4(my_proc *proc, int n) { // Round Robin scheduling 함수 time quantum = 4
	int p_num = 0;
	int count = 0;

	My_sort(proc, n);

	my_queue que;
	my_queue_init(&que);

	printf("RR(q=4) : ");

	while(1) {
		int time_q=0;
		if(p_num<5) {
			if(proc[p_num].arri_t == count) {
				my_queue_insert(&que, proc[p_num]);
				p_num++;
			}
		}
		if(my_queue_is_empty(&que)) {
			break;
		}
		my_proc ret_proc = my_queue_delete(&que);
		while(ret_proc.cpu_t>0) {
			if(p_num<5) {
				if(proc[p_num].arri_t == count) {
					my_queue_insert(&que, proc[p_num]);
					p_num++;
				}
			}
			if(my_queue_is_empty(&que)==0 && time_q>=4) { // My_RRq1에서 time quantum 만 4로 변경
				my_queue_insert(&que, ret_proc);
				break;
			}
			printf("%c ", ret_proc.p_name);
			ret_proc.cpu_t--;
			count++;
			time_q++;
		}
	}
	printf("\n");
}

void My_SJF(my_proc *proc, int n) { //Shortest Job First scheduling 함수
	int p_num = 0;
	int count = 0;

	My_sort(proc, n);

	my_queue que;
	my_queue_init(&que);

	printf("SJF : ");
	
	while(1) {
		if(p_num<5) {
			if(proc[p_num].arri_t == count) {
				my_queue_insert(&que, proc[p_num]);
				p_num++;
			}
		}
		if(my_queue_is_empty(&que)) {
			break;
		}

		SJF_queue_sort(&que); // 레디 큐에 있는 프로세스 중 수행 시간이 가장 짧은 프로세스 먼저 수행하기 위해
				     // 프로세스 위치 이동

		my_proc ret_proc = my_queue_delete(&que);
		while(ret_proc.cpu_t>0) {
			if(p_num<5) {
				if(proc[p_num].arri_t == count) {
					my_queue_insert(&que, proc[p_num]);
					p_num++;
				}
			}
			printf("%c ", ret_proc.p_name);
			ret_proc.cpu_t--;
			count++;
		}
	}
	printf("\n");
}

void My_MLFQq1(my_proc *proc, int n) { // Multi Level Feedback Queue scheduling 함수 time quantum = 1
	int p_num = 0;
	int count = 0;

	My_sort(proc, n);

	my_queue que[5]; // 레디 큐를 5개의 레벨로 구성, que[0] 부터 que[5] 까지 갈수록 우선순위가 낮아짐
	for(int i=0; i<5; i++) {
		my_queue_init(&que[i]); // 레디 큐 초기화
	}

	printf("MLFQ(q=1) : ");

	while(1) {
		int time_q = 0;
		if(p_num<5) {
			if(proc[p_num].arri_t == count) {
				my_queue_insert(&que[0], proc[p_num]);
				p_num++;
			}
		}
		int is_empty = 0; // 레디 큐가 비어있는지 확인하기 위한 변수
		for(int i=0; i<5; i++) {
			if(my_queue_is_empty(&que[i])){
				is_empty++; // 5개의 레디 큐 중 비어있는 레디 큐가 있다면 is_empty + 1
			}
			else {
				break; // 비어있지 않은 레디 큐가 있다면 해당 레디 큐의 프로세스 수행 시작
			}
		}
		if(is_empty==5) {
			break; // 모든 레디 큐가 비어있다면 스케줄링 종료
		}

		my_proc ret_proc = my_queue_delete(&que[is_empty]);
			// 비어있지 않은 레디 큐에서 우선순위가 가장 높은 레디 큐에 있는 프로세스 수행을 위한 삭제
		while(ret_proc.cpu_t>0) {
			if(p_num<5) {
				if(proc[p_num].arri_t == count) {
					my_queue_insert(&que[0], proc[p_num]);
					p_num++;
				}
			}
			if(time_q>=1) { // 해당 프로세스가 자신의 time quantum 만큼 수행 했는데
					// 레디 큐에 기다리고 있는 프로세스가 없고 수행 시간이 끝나지 않았다면
					// 계속해서 수행
				is_empty = 0;
				for(int i=0; i<5; i++) {
					if(my_queue_is_empty(&que[i])) {
						is_empty++;
					}
					else {
						break;
					}
				}
				if(is_empty<5) {
						// 만약 레디 큐에 기다리는 프로세스가 있다면
						// 수행중이던 프로세스의 우선순위를 한 단계 낮추고(+1)
						// 그 우선순위에 해당하는 레디 큐에 삽입(우선순위는 0~4)
						// 우선순위가 4 라면 마지막 레디 큐에서 수행중인 프로세스이므로 그대로 유지
					if(ret_proc.p_priority < 4) {
						ret_proc.p_priority++;
					}
					my_queue_insert(&que[ret_proc.p_priority], ret_proc);
					break;
				}
			}
			printf("%c ", ret_proc.p_name);
			ret_proc.cpu_t--;
			count++;
			time_q++;
		}
	}
	printf("\n");
}

void My_MLFQq2(my_proc *proc, int n) { //Multi Level Feedback Queue scheduling 함수 time quantum = 2^i
	int p_num = 0;
	int count = 0;

	My_sort(proc, n);

	my_queue que[5];
	for(int i = 0; i<5; i++) {
		my_queue_init(&que[i]);
	}
	
	printf("MLFQ(q=2^i) : ");

	while(1) {
		int time_q = 0;
		if(p_num<5) {
			if(proc[p_num].arri_t == count) {
				my_queue_insert(&que[0], proc[p_num]);
				p_num++;
			}
		}
		
		int is_empty = 0;
		for(int i=0; i<5; i++) {
			if(my_queue_is_empty(&que[i])) {
				is_empty++;
			}
			else {
				break;
			}
		}
		if(is_empty==5) {
			break;
		}
		
		my_proc ret_proc = my_queue_delete(&que[is_empty]);
		while(ret_proc.cpu_t>0) {
			if(p_num<5) {
				if(proc[p_num].arri_t == count) {
					my_queue_insert(&que[0], proc[p_num]);
					p_num++;
				}
			}
			if(time_q >= my_pow(2,is_empty)) { // n제곱 함수를 사용하여 해당 프로세스의 time quantum 계산
				int is_empty2 = 0;
				for(int i=0; i<5; i++) {
					if(my_queue_is_empty(&que[i])) {
						is_empty2++;
					}
					else {
						break;
					}
				}
				if(is_empty2<5) {
					if(ret_proc.p_priority < 4) {
						ret_proc.p_priority++;
					}
					my_queue_insert(&que[ret_proc.p_priority], ret_proc);
					break;
				}
			}
			printf("%c ", ret_proc.p_name);
			ret_proc.cpu_t--;
			count++;
			time_q++;
		}
	}
	printf("\n");
}
