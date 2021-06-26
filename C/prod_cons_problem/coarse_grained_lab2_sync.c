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
CQ c_queue; // 차량 생산 큐 전역 번수로 선언(생산자, 소비자 스레드가 공유하기 때문)
pthread_cond_t empty = PTHREAD_COND_INITIALIZER;
pthread_cond_t fill = PTHREAD_COND_INITIALIZER;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
int count = 0; // 생산된 차량의 수(현재 시각)
int time_quantum = 0; // 1 or 4
int total_car = 0; // 100 or 1000 or 10000

void RQ_init(RQ *r_queue) { // 레디 큐 생성자 함수
	Node *node = malloc(sizeof(Node));
		/* 원형 큐로 구현하기 대문에 큐가 비어있는 상태를 나타내기 위한
		   빈 노드 생성 (front와 rear가 같은 노드를 가리키면 큐는 비어있는 상태) */
	node->next = NULL;
	r_queue->front = r_queue->rear = node; // 초기 큐는 비어있는 상태
}

void RQ_insert(RQ *r_queue, Node node) { // 레디 큐 삽입 함수
	Node *temp = malloc(sizeof(Node)); // 삽입할 데이터를 담을 노드
	memcpy(temp, &node, sizeof(Node)); // 노드에 데이터 복사
	r_queue->front->next = temp; // 큐에 노드 추가
	r_queue->front = temp; // front 위치 이동
}

Node RQ_delete(RQ *r_queue) { // 레디 큐 삭제 함수
	Node *temp = r_queue->rear; // 삽입 함수에서 동적 생성된 노드를 삭제하기 위한 노드
	Node del_node; // 삭제할 데이터를 반환하기 위한 노드
	memcpy(&del_node, temp->next, sizeof(Node)); // 반환을 위한 노드에 삭제할 데이터 복사
	r_queue->rear = temp->next; // rear 위치 이동
	free(temp); // 메모리 반납
	return del_node; // 삭제한 데이터 반환
}

int RQ_is_empty(RQ *r_queue) { // 레디 큐 empty case check
	if(r_queue->front == r_queue->rear) {
		return 1; // 레디 큐가 비어 있다면 1 반환
	}
	else {
		return 0; // 비어있지 않으면 0 반환
	}
}

void CQ_init(CQ *c_queue) { // 차량 생산 큐 생성자 함수
	c_queue->balance = 0; // 현재 차량 생산 큐에 있는 차량의 수 0 개
	Node *temp = malloc(sizeof(Node));
	temp->next = NULL;
	c_queue->front = c_queue->rear = temp; // 레디 큐 생성자 함수와 동일(초기 큐는 비어있는 상태)
}

void CQ_insert(CQ *c_queue, Node node) { // 차량 생산 큐 삽입 함수
	Node *temp = malloc(sizeof(Node));
	memcpy(temp, &node, sizeof(Node));
	c_queue->front->next = temp;
	c_queue->front = temp;
		// 레디 큐 삽입 함수와 동일한 패턴
	c_queue->balance++; // 차량 생산 큐에 있는 차량의 수 1 증가
}

void CQ_delete(CQ *c_queue, int n) { // 차량 생산 큐 삭제 함수
	if(c_queue->rear->next->car_num == n) {
			// 소비자가 구매할 수 있는 차량이어야만 구매(삭제) 수행
		Node *temp = c_queue->rear;
		c_queue->rear = temp->next;
		c_queue->balance--; // 차량 생산 큐의 차량 수 1 감소
		free(temp);
	}
	else {
		pthread_cond_wait(&fill, &lock); // 소비자가 구매할 수 없는 차량이면 해당 소비자 스레드는 sleep 상태 돌입
	}
}

void *producer(void *arg) { // 생산자 차량 생산 함수, 전달 인자로 차량 노드 배열을 받음
	Node *car = (Node *) arg;
	int car_n = 0; // 레디 큐에 삽입되지 않은 차량 중에서 생산 시작 시간이 가장 빠른 차량의 인덱스

	RQ r_queue;
	RQ_init(&r_queue); // 레디 큐 초기화
	CQ_init(&c_queue); // 차량 생산 큐 
	
	while(1) { // RR 방식 생산 시작
		int time_q=0; // time quantum 변수
		if(car_n<5) { // 오버플로우 방지를 위한 조건
			if(car[car_n].start_t == count) { // count는 차량 생산 수 및 현재 시각
				RQ_insert(&r_queue, car[car_n]); // 해당 차량의 생산 시작 시간이
								// 현재 시각과 같다면 레디 큐에 삽입하여 생산 시작
				car_n++;
			}
		}
		if(RQ_is_empty(&r_queue)) {
			break; // 레디 큐가 비어있다면 차량 생산이 모두 종료된 것이므로 생산 종료
		}
		Node ret_node = RQ_delete(&r_queue);
			// 차량 생산 큐에 삽입하기 위해 레디 큐에서 삭제 수행
		while(ret_node.prod_num>0) { // 생산량 만큼 반복
			if(car_n<5) { // 오버플로우 방지를 위한 조건
				if(car[car_n].start_t == count) {
					RQ_insert(&r_queue, car[car_n]);
					car_n++;
				}
			}
			if(RQ_is_empty(&r_queue)==0 && time_q>=time_quantum) {
				RQ_insert(&r_queue, ret_node);
				break; // 레디 큐에 차량(노드)이 존재하고 time quantum이 1(or 4) 이상이면
					// 현재 차량을 레디 큐의 마지막에 다시 추가하고 대기중인 다음 차량 생산 시작
			}
			pthread_mutex_lock(&lock); // 생산 큐의 full case 처리를 위한 lock
			while(c_queue.balance == 10) {
				pthread_cond_wait(&empty, &lock);
			}
			CQ_insert(&c_queue, ret_node); // 차량을 차량 생산 큐에 삽입(출고)
			count++; // 생산된 차량의 수(현재 시각) 1 증가
			pthread_cond_broadcast(&fill); // 모든 소비자들을 깨우기 위해 broadcast 사용
			pthread_mutex_unlock(&lock);
			ret_node.prod_num--; // 해당 차량 재고 1 감소
			time_q++; // time quantum 1 증가
		}
	}				
}

void *consumer(void *arg) { // 소비자 차량 구매 함수, 전달 인자로 소비자 이름을 받음(문자열)
	while(count<total_car) { // 차량 생산이 끝나면 수행 종료
		pthread_mutex_lock(&lock); // 차량 생산 큐의 empty case를 처리하기 위한 lock
		while(c_queue.balance == 0) {
			pthread_cond_wait(&fill, &lock);
		}
		if(strcmp((char *)arg, "C_a")==0) { // 전달 인자로 받은 문자열과 소비자 이름을 비교
			CQ_delete(&c_queue, 0); // 소비자 C_a는 차량 번호 0번만 구매 가능
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
		pthread_cond_signal(&empty); // 생산자를 깨움
		pthread_mutex_unlock(&lock);
	}
}

void lab2_sync_usage(char *cmd) { // 바이너리 실행 시 예외 처리 함수
	printf("\n Usage for %s : \n",cmd);
    printf("    -c: Total number of vehicles produced, must be bigger than 0 ( e.g. 100 )\n");
    printf("    -q: RoundRobin Time Quantum, must be bigger than 0 ( e.g. 1, 4 ) \n");
}

void lab2_sync_example(char *cmd) { // 바이너리 실행 시 예외 처리 함수
	printf("\n Example : \n");
    printf("    #sudo %s -c=100 -q=1 \n", cmd);
    printf("    #sudo %s -c=10000 -q=4 \n", cmd);
}

int main(int argc, char* argv[]) { // 메인 함수
	char op;
	int n; char junk;
	struct timeval start, end; // 수행 시간 출력을 위한 구조체 변수 선언
	double result_t;


	if (argc <= 1) { // 바이너리 실행 시 전달 인자가 1개 이하라면 예외 처리 함수 실행 후 종료
		lab2_sync_usage(argv[0]);
		lab2_sync_example(argv[0]);
		exit(0);
	}

	for (int i = 1; i < argc; i++) {
		if (sscanf(argv[i], "-c=%d%c", &n, &junk) == 1) { // 바이너리 실행 시 argv[0]로 전달한 생산할 총 차량의 수를 total_car에 저장
			total_car = n;
		}
		else if (sscanf(argv[i], "-q=%d%c", &n, &junk) == 1) { // 바이너리 실행 시 argv[1]로 전달한 time quantum을 time_quantum에 저장
			time_quantum = n;
		}
		else { // 예외 처리 후 종료
			lab2_sync_usage(argv[0]);
			lab2_sync_example(argv[0]);
			exit(0);
		}
	}
	gettimeofday(&start, NULL);
	
	Node car[5] = { {0, 0, 15, NULL}, {1, 2, 20, NULL}, {2, 4, 10, NULL}, {3, 6, 40, NULL}, {4, 8, 15, NULL} };
	pthread_t p, c1, c2, c3, c4, c5;
	pthread_create(&p, NULL, producer, (void *) car); // 생산자 스레드 생성
	pthread_create(&c1, NULL, consumer, "C_a");
	pthread_create(&c2, NULL, consumer, "C_b");
	pthread_create(&c3, NULL, consumer, "C_c");
	pthread_create(&c4, NULL, consumer, "C_d");
	pthread_create(&c5, NULL, consumer, "C_e"); // 소비자 스레드 5개 생성
	/*
	pthread_create(&c6, NULL, consumer, "C_a");
	pthread_create(&c7, NULL, consumer, "C_b");
	pthread_create(&c8, NULL, consumer, "C_c");
	pthread_create(&c9, NULL, consumer, "C_d");
	pthread_create(&c10, NULL, consumer, "C_e");
	*/
	pthread_join(p, NULL); // 생산자 스레드 먼저 수행
	gettimeofday(&end, NULL);
	result_t = (end.tv_sec - start.tv_sec) + ((end.tv_usec - start.tv_usec)/1000000); // 수행 시간 
	
	printf("Coarse-grained Experiment\n");
	printf("Total Produce Number : %d\n", count);
	printf("Final Balance Value : %d\n", c_queue.balance);
	printf("Execution Time : %f\n", result_t);
	
	
	return 0;
}
