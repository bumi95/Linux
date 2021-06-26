/*
*   DKU Operating System Lab
*           Lab2 (Vehicle production Problem)
*           Student id : 32141925
*           Student name : Park Jong Bum
*
*   lab2_sync.c :
*       - lab2 main file.
*       - must contains Vehicle production Problem function's declations.
*
*/

#include <aio.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <assert.h>
#include <pthread.h>
#include <asm/unistd.h>

#include "lab2_sync_types.h"

/*
 * you need to implement Vehicle production Problem. 
 */
CQ c_queue;
pthread_cond_t empty = PTHREAD_COND_INITIALIZER;
pthread_cond_t fill = PTHREAD_COND_INITIALIZER;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
int count = 0;
int time_quantum = 0;
int total_car = 0;

void RQ_init(RQ *r_queue) {
	Node *node = malloc(sizeof(Node));
	node->next = NULL;
	r_queue->front = r_queue->rear = node;
}

void RQ_insert(RQ *r_queue, Node node) {
	Node *temp = malloc(sizeof(Node));
	memcpy(temp, &node, sizeof(Node));
	r_queue->front->next = temp;
	r_queue->front = temp;
}

Node RQ_delete(RQ *r_queue) {
	Node *temp = r_queue->rear;
	Node del_node;
	memcpy(&del_node, temp->next, sizeof(Node));
	r_queue->rear = temp->next;
	free(temp);
	return del_node;
}

int RQ_is_empty(RQ *r_queue) {
	if(r_queue->front == r_queue->rear) {
		return 1;
	}
	else {
		return 0;
	}
}

void CQ_init(CQ *c_queue) {
	c_queue->balance = 0;
	Node *temp = malloc(sizeof(Node));
	temp->next = NULL;
	c_queue->front = c_queue->rear = temp;
}

void CQ_insert(CQ *c_queue, Node node) {
	Node *temp = malloc(sizeof(Node));
	memcpy(temp, &node, sizeof(Node));

	//pthread_mutex_lock(&lock);
	c_queue->front->next = temp;
	c_queue->front = temp;
	c_queue->balance++;
	//pthread_mutex_unlock(&lock);
}

void CQ_delete(CQ *c_queue, int n) {
	/*if(c_queue->balance==0) {
		return;
	}*/
	if(c_queue->rear->next->car_num == n) {
		//pthread_mutex_lock(&lock);
		Node *temp = c_queue->rear;
		c_queue->rear = temp->next;
		c_queue->balance--;
		free(temp);
		//pthread_mutex_unlock(&lock);
	}
	else {
		//pthread_mutex_lock(&lock);
		pthread_cond_wait(&fill, &lock);
		//pthread_mutex_unlock(&lock);
	}
}

void *producer(void *arg) {
	Node *car = (Node *) arg;
	int car_n = 0;

	RQ r_queue;
	RQ_init(&r_queue);
	CQ_init(&c_queue);
	
	while(1) {
		int time_q=0;
		if(car_n<5) {
			if(car[car_n].start_t == count) {
				RQ_insert(&r_queue, car[car_n]);
				car_n++;
			}
		}
		if(RQ_is_empty(&r_queue)) {
			break;
		}
		Node ret_node = RQ_delete(&r_queue);
		while(ret_node.prod_num>0) {
			if(car_n<5) {
				if(car[car_n].start_t == count) {
					RQ_insert(&r_queue, car[car_n]);
					car_n++;
				}
			}
			if(RQ_is_empty(&r_queue)==0 && time_q>=time_quantum) {
				RQ_insert(&r_queue, ret_node);
				break;
			}
			pthread_mutex_lock(&lock);
			while(c_queue.balance == 10) {
				pthread_cond_wait(&empty, &lock);
			}
			//pthread_cond_broadcast(&fill);
			//pthread_mutex_unlock(&lock);
			CQ_insert(&c_queue, ret_node);
			count++;
			pthread_cond_broadcast(&fill);
			pthread_mutex_unlock(&lock);
			ret_node.prod_num--;
			time_q++;
		}
	}				
}

void *consumer(void *arg) {
	while(count<total_car) {
		pthread_mutex_lock(&lock);
		while(c_queue.balance == 0) {
			pthread_cond_wait(&fill, &lock);
		}
		//pthread_cond_signal(&empty);
		//pthread_mutex_unlock(&lock);
		if(strcmp((char *)arg, "C_a")==0) {
			CQ_delete(&c_queue, 0);
		}
		else if(strcmp((char *)arg, "C_b")==0) {
			CQ_delete(&c_queue, 1);
		}
		else if(strcmp((char *)arg, "C_c")==0) {
			CQ_delete(&c_queue, 2);
		}
		else if(strcmp((char *)arg, "C_d")==0) {
			CQ_delete(&c_queue, 3);
		}
		else if(strcmp((char *)arg, "C_e")==0) {
			CQ_delete(&c_queue, 4);
		}
		else {
			printf("error\n");
		}
		pthread_cond_signal(&empty);
		pthread_mutex_unlock(&lock);
	}
}

void lab2_sync_usage(char *cmd) {
	printf("\n Usage for %s : \n",cmd);
    printf("    -c: Total number of vehicles produced, must be bigger than 0 ( e.g. 100 )\n");
    printf("    -q: RoundRobin Time Quantum, must be bigger than 0 ( e.g. 1, 4 ) \n");
}

void lab2_sync_example(char *cmd) {
	printf("\n Example : \n");
    printf("    #sudo %s -c=100 -q=1 \n", cmd);
    printf("    #sudo %s -c=10000 -q=4 \n", cmd);
}

int main(int argc, char* argv[]) {
	char op;
	int n; char junk;
	struct timeval start, end;
	double result_t;


	if (argc <= 1) {
		lab2_sync_usage(argv[0]);
		lab2_sync_example(argv[0]);
		exit(0);
	}

	for (int i = 1; i < argc; i++) {
		if (sscanf(argv[i], "-c=%d%c", &n, &junk) == 1) {
			total_car = n;
		}
		else if (sscanf(argv[i], "-q=%d%c", &n, &junk) == 1) {
			time_quantum = n;
		}
		else {
			lab2_sync_usage(argv[0]);
			lab2_sync_example(argv[0]);
			exit(0);
		}
	}
	gettimeofday(&start, NULL);
	
	Node car[5] = { {0, 0, 15000, NULL}, {1, 2, 20000, NULL}, {2, 4, 10000, NULL}, {3, 6, 40000, NULL}, {4, 8, 15000, NULL} };
	pthread_t p, c1, c2, c3, c4, c5, c6, c7, c8, c9, c10;
	pthread_create(&p, NULL, producer, (void *) car);
	pthread_create(&c1, NULL, consumer, "C_a");
	pthread_create(&c2, NULL, consumer, "C_b");
	pthread_create(&c3, NULL, consumer, "C_c");
	pthread_create(&c4, NULL, consumer, "C_d");
	/*pthread_create(&c5, NULL, consumer, "C_e");
	pthread_create(&c6, NULL, consumer, "C_a");
	pthread_create(&c7, NULL, consumer, "C_b");
	pthread_create(&c8, NULL, consumer, "C_c");
	pthread_create(&c9, NULL, consumer, "C_d");
	pthread_create(&c10, NULL, consumer, "C_e");
	*/
	pthread_join(p, NULL);
	gettimeofday(&end, NULL);
	result_t = (end.tv_sec - start.tv_sec) + ((end.tv_usec - start.tv_usec)/1000000);
	
	printf("Coarse-grained Experiment\n");
	printf("Total Produce Number : %d\n", count);
	printf("Final Balance Value : %d\n", c_queue.balance);
	printf("Execution Time : %f\n", result_t);
	
	
	return 0;
}
