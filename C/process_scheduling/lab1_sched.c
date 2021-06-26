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
void my_queue_init(my_queue *que) {

	que->front = que->RQ;
	que->rear = que->RQ;
}

void my_queue_insert(my_queue *que, my_proc proc) {

	if(que->RQ+6 == que->front) {
		que->front = que->RQ;
	}
	memcpy(que->front, &proc, sizeof(my_proc));
	que->front++;
}

my_proc my_queue_delete(my_queue *que) {

	if(que->RQ+6 == que->rear) {
		que->rear = que->RQ;
	}
	my_proc temp;
	memcpy(&temp, que->rear, sizeof(my_proc));
	que->rear++;
	return temp;
}

int my_queue_is_empty(my_queue *que) {

	if(que->front == que->rear) {
		return 1;
	}
	else {
		return 0;
	}
}

void My_sort(my_proc *proc, int n) {
	my_proc temp;
	int i, j;
	
	for(i=0; i<n; i++) {
		for(j=i+1; j<n; j++) {
			if(proc[i].arri_t > proc[j].arri_t) {
				temp = proc[j];
				proc[j] = proc[i];
				proc[i] = temp;
			}
			else if(proc[i].arri_t == proc[j].arri_t) {
				if(proc[i].p_name > proc[j].p_name) {
					temp = proc[j];
					proc[j] = proc[i];
					proc[i] = temp;
				}
			}
		}
	}
}

int my_pow(int n, int m) {
	int res = 1;
	while(m>0) {
		res *= n;
		m--;
	}
	return res;
}

void SJF_queue_sort(my_queue *que) {
	my_proc *temp;
	if(que->rear+1 == que->RQ+6) {
		temp = que->RQ;
	}
	else {
		temp = que->rear+1;
	}
	while(que->front != temp) {
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

void My_FCFS(my_proc *proc, int n){
	int p_num = 0;
	int count = 0;

	My_sort(proc, n);

	my_queue que;
	my_queue_init(&que);

	printf("FCFS : ");
	
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

void My_RRq1(my_proc *proc, int n) {
	int p_num = 0;
	int count = 0;

	My_sort(proc, n);
	
	my_queue que;
	my_queue_init(&que);

	printf("RR(q=1) : ");

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
			if(my_queue_is_empty(&que)==0 && time_q>=1) {
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

void My_RRq4(my_proc *proc, int n) {
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
			if(my_queue_is_empty(&que)==0 && time_q>=4) {
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

void My_SJF(my_proc *proc, int n) {
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

		SJF_queue_sort(&que);

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

void My_MLFQq1(my_proc *proc, int n) {
	int p_num = 0;
	int count = 0;

	My_sort(proc, n);

	my_queue que[5];
	for(int i=0; i<5; i++) {
		my_queue_init(&que[i]);
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
		int is_empty = 0;
		for(int i=0; i<5; i++) {
			if(my_queue_is_empty(&que[i])){
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
			if(time_q>=1) {
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

void My_MLFQq2(my_proc *proc, int n) {
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
			if(time_q >= my_pow(2,is_empty)) {
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
