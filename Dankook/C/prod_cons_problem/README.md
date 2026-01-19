생산자 소비자 문제 구현
=========================
## 문제 정의
> 생산자 소비자 문제에 대해 정의합니다.
* 공장에서 **5종류의 차량을 생산**하고 **생산 라인은 하나**만 존재합니다.
* **차량 생산자**는 생산 라인에 차량을 **Round Robin 방식으로 출고**를 진행합니다.
* **5명의 소비자들**은 생산 라인에 출고된 차량을 **앞에서부터 하나씩 구매**할 수 있습니다.
* 생산 라인에 **출고된 차량이 없는 경우** 소비자는 차량 **구매가 불가**합니다.
* 생산 라인이 **가득 찬 경우** 생산자는 차량 **출고가 불가**합니다.
* time quantum = 1, 4
* 생산 라인에 들어갈 수 있는 **차량의 최대 수는 10대**이고, 차량 count가 음수일 수는 없습니다.
* 소비자 **C_a는 차량 V_a만 구매가 가능**합니다. (차량은 숫자로 표현해도 무관합니다.)
* 각 차량의 **생산 수는 랜덤 또는 미리 지정**이 가능합니다.
* 전체 차량 **생산 수는 100, 1000, 10000 등 증가**시키며 실험을 진행합니다.
* 5종류의 **차량의 각 생산 수를 합친 것이 전체 차량의 생산 수** 입니다.
* 총 실행 시간(gettimeofday 사용)을 출력합니다.
* 최종 프로그램 실행 후 balance 값을 출력합니다. (balance = 생산 라인에 남아있는 차량의 수)
* 전체 차량의 생산 수와 lock 여부에 따른 위 2가지 결과 값을 출력합니다.
* 스레드 개수 변화에 따른 성능을 분석합니다.
* coarse-garined와 fine-grained인 경우에 따라 성능 변화를 분석합니다.
## 문제 설계
> 문제 정의에서 강조한 부분에 대한 기초 설계부터 진행하였습니다.
* 5종류의 차량 생산 : 차량 노드 배열
* 생산 라인 : 차량 생산 큐
* RR 방식으로 출고 : RR 방식 생산을 위한 레디 큐와 RR 알고리즘 구현 및 차량 생산 큐의 삽입 함수
* 생산자 : 생산자 스레드와 생산자 차량 생산 함수
* 5명의 소비자 : 소비자 스레드 5개와 소비자 차량 구매 함수
* 앞에서부터 하나씩 구매 : 차량 생산 큐의 삭제 함수
* 출고된 차량이 없는 경우 구매 불가 : 차량 생산 큐의 empty case
* 생산 라인이 가득 찬 경우 생산 불가 : 차량 생산 큐의 full case
* 차량의 최대 수는 10대 : 차량 생산 큐의 크기는 10
* 소비자 C_a는 차량 V_a만 구매 가능 : 소비자 차량 구매 함수에서 제약조건 구현
* 생산 수는 랜덤 또는 미리 지정 : 메인 함수에서 미리 지정하는 것으로 구현 

차량 생산 큐, lock 변수, 조건 변수, 전체 차량의 생산 수는 구매자와 생산자가 공유하므로 전역 변수로 선언합니다.

전체 차량 생산 수의 변화는 메인 함수에서 지정하고 lock은 No-lock, coarse-grained and fine-grained 3가지로 구분하여 구현합니다.   
코드 상에서 3가지 경우 동시에 구현하지 않고 구현된 코드에서 3가지 경우로 각각 코드를 수정하여 결과값을 도출합니다.   

조건 변수는 차량 생산 큐의 empty case와 full case의 경우에 스레드 사이의 문맥 교환을 위하여 사용합니다.

스레드의 개수는 생산자는 1명이므로 1개로 고정하고 소비자 스레드를 늘리는 방식으로 스레드 개수 변화에 따른 성능 분석을 합니다.   
또한, pthread_join 함수를 이용하여 프로그램 수행 시 항상 생산자 스레드가 먼저 실행되도록 합니다.
## 구현 및 결과
> *No-lock*의 경우
* 스레드 사이의 문맥 교환을 위한 lock을 제외한 모든 lock을 해제합니다.
> *Coarse-grained*의 경우
* 임계 영역에 대해 큰 단위로 lock을 설정합니다.
> *Fine-grained*의 경우
* 임계 영역에 대해 작은 단위로 lock을 설정합니다.
### 임계 영역
> 임계 영역은 공유 자원을 접근 하는 영역입니다.   
> 차량 생산 큐 삽입 및 삭제 함수, 전체 차량의 생산 수를 접근 하는 영역입니다.
```c
pthread_mutex_lock(&lock); // 차량 생산 큐의 full case 처리를 위한 lock
while(c_queue.balance == 10) {
	pthread_cond_wait(&empty, &lock);
}
pthread_cond_broadcast(&fill); // 모든 소비자를 깨우기 위해 broadcast 사용
pthread_mutex_unlock(&lock);
CQ_insert(&c_queue, ret_node); // 차량을 차량 생산 큐에 삽입(출고)
count++; // 생산된 차량 수(현재 시각) 1 증가
ret_node.prod_num--; // 해당 차량 재고 1 감소
time_q++; // time quantum 1 증가
```
> 위의 코드는 No-lock 생산자 차량 생산 함수의 일부 입니다.   
> 큐의 full case 처리 부분(스레드 문맥 교환)을 제외하고 임계 영역에 대한 lock을 설정하지 않습니다.
```c
pthread_mutex_lock(&lock); // 차량 생산 큐의 empty case를 처리하기 위한 lock
while(c_queue.balance == 0) {
	pthread_cond_wait(&fill, &lock);
}
pthread_cond_signal(&empty); // 생산자를 깨움
pthread_mutex_unlock(&lock);
if(strcmp((char *)arg, "C_a")==0) { // 전달 인자로 받은 문자열과 소비자 이름을 비교
	CQ_delete(&c_queue, 0); // 소비자 C_a는 차량 번호 0번만 구매 가능
}
```
> 위의 코드는 No-lock 소비자 차량 구매 함수의 일부 입니다.   
> 큐의 empty case 처리 부분(스레드 문맥 교환)을 제외하고 임계 영역에 대한 lock을 설정하지 않습니다.
```c
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
```
> 위의 코드는 Coarse-grained 생산자 차량 생산 함수의 일부 입니다.   
> 스레드 문맥 교환 부분과 차량 생산 큐 삽입 함수, 전체 차량 생산 수 증가 부분을 한꺼번에 묶어 lock을 설정합니다.
```c
pthread_mutex_lock(&lock); // 차량 생산 큐의 empty case를 처리하기 위한 lock
while(c_queue.balance == 0) {
	pthread_cond_wait(&fill, &lock);
}
if(strcmp((char *)arg, "C_a")==0) { // 전달 인자로 받은 문자열과 소비자 이름을 비교
	CQ_delete(&c_queue, 0); // 소비자 C_a는 차량 번호 0번만 구매 가능
}

...

else if(strcmp((char *)arg, "C_e")==0) {
	CQ_delete(&c_queue, 4);
}
else {
	printf("error\n");
}
pthread_cond_signal(&empty); // 생산자를 깨움
pthread_mutex_unlock(&lock);
```
> 위의 코드는 Coarse-grained 소비자 차량 구매 함수의 일부 입니다.   
> 스레드의 문맥 교환 부분과 차량 생산 큐 삭제 함수 부분을 한꺼번에 묶어 lock을 설정합니다.
```c
pthread_mutex_lock(&lock);
c_queue->front->next = temp;
c_queue->front = temp;
	// 레디 큐 삽입 함수와 동일한 패턴
c_queue->balance++; // 차량 생산 큐에 있는 차량의 수 1 증가
pthread_mutex_unlock(&lock);
```
> 위의 코드는 Fine-grained 차량 생산 큐 삽입 함수의 일부 입니다.   
> 삽입 함수 내부에 lock을 설정하여 좁은 범위의 lock을 구현합니다.
```c
if(c_queue->rear->next->car_num == n) {
		// 소비자가 구매할 수 있는 차량이어야만 구매(삭제) 수행
	pthread_mutex_lock(&lock);
	Node *temp = c_queue->rear;
	c_queue->rear = temp->next;
	c_queue->balance--; // 차량 생산 큐의 차량 수 1 감소
	free(temp);
	pthread_mutex_unlock(&lock);
}
else {
	pthread_mutex_lock(&lock);
	pthread_cond_wait(&fill, &lock); // 소비자가 구매할 수 없는 차량이면 해당 소비자 스레드는 sleep 상태 돌입
	pthread_mutex_unlock(&lock);
}
```
> 위의 코드는 Fine-grained 차량 생산 큐 삭제 함수의 일부 입니다.   
> 삭제 함수 내부에 삭제를 수행하는 부분과 수행하지 않고 sleep 상태로 들어가는 부분에 각각 lock을 설정하여 좁은 범위의 lock을 구현합니다.
### 수행 결과
> 차량 수에 따른 변화, lock의 유무에 따른 변화, 스레드 개수에 따른 변화 등 출력 결과의 종류가 매우 다양하기 때문에 간단하게 몇 개만 표시합니다.   
>    
![c10000q1nolock](https://user-images.githubusercontent.com/39798011/123538297-2f4f4d00-d76f-11eb-9f32-5050c9bf20b5.png)
> 위 그림은 차량 수 10000개, time quantum = 1 인 경우 no-lock에 해당하는 출력 결과입니다.   
> 실행 시간이 굉장히 오래 걸리는 것을 볼 수 있습니다.   
>    
![c10000q1coarse_grained](https://user-images.githubusercontent.com/39798011/123538352-70dff800-d76f-11eb-8049-4133efade1b6.png)
> 위 그림은 같은 조건에 coarse-grained에 해당하는 출력 결과 입니다.   
> 실행 시간이 0초에 수렴합니다.   
>    
![c10000q1fine_grained](https://user-images.githubusercontent.com/39798011/123538392-a84ea480-d76f-11eb-9f6e-8415ef4e8c99.png)
> 위 그림은 같은 조건에 fine-grained에 해당하는 출력 결과 입니다.   
> 실행 시간이 0초에 수렴합니다.   
>    
![thread5_coarse_grained](https://user-images.githubusercontent.com/39798011/123538430-d8964300-d76f-11eb-9cb8-0d81206d09cb.png)
> 위 그림은 차량 수 100000개, time quantum = 1 인 경우 coarse-grained에 해당하는 출력 결과입니다.   
> 실행 시간이 약 1초 정도 소요 됩니다.   
>    
![thread10_coarse_grained](https://user-images.githubusercontent.com/39798011/123538471-08dde180-d770-11eb-98b2-0e228d78fc7c.png)
> 위 그림은 같은 조건에 소비자 스레드의 개수를 10개로 늘린 경우에 해당하는 출력 결과입니다.   
> 실행 시간이 약 1~2초 정도 소요 되는데 스레드 사이의 문맥 교환에 의하여 시간이 더 걸리는 것으로 생각됩니다.
