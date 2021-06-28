프로세스 스케줄링 시뮬레이터 구현
=================================
## 문제 정의
> 프로세스 스케줄링 FCFS, SJF, RR, MLFQ 에 대한 시뮬레이터를 구현합니다.   
> 실제 프로세스를 생성하지는 않고 스케줄링 알고리즘의 결과만을 확인합니다.   
> ubuntu linux 환경에서 수행합니다.
## 문제 설계
> 헤더 파일
* 프로세스를 나타낼 구조체
    + 멤버 변수로 프로세스 이름, 수행 시간, 도착 시간 및 우선 순위를 가집니다.
* 레디 큐 구조체
    + 최대 5개의 프로세스가 들어갈 원형 큐로 구현하며 full, empty case를 구별하기 위해 큐의 크기를 6으로 설정합니다.
* 레디 큐 생성, 삽입, 삭제 및 empty case 함수 선언부
* 스케줄링 알고리즘 및 관련 함수 선언부
> 스케줄링 알고리즘 구현 파일
* 레디 큐 관련 함수, 스케줄링 알고리즘 함수 및 관련 함수 구현부
    + 레디 큐 생성, 삽입, 삭제 및 empty case 함수
    + 프로세스 도착 시간 순 정렬 함수
    + SJF 스케줄링을 위한 수행 시간이 가장 짧은 프로세스 순 정렬 함수
    + MLFQ 스케줄링 time quantum = 2^i 표현을 위한 n제곱 함수
    + 스케줄링 알고리즘 함수
> 스케줄링 테스트 파일
* 메인 함수 및 워크로드 설정
    + 메인 함수에 프로세스들의 워크로드를 설정하고 각 스케줄링 알고리즘을 수행시킵니다.
> 작성한 3개의 파일은 주어진 makefile을 이용하여 컴파일 합니다.
## 구현 및 결과
### First Come First Served
> 각 스케줄링 함수의 기본 구조는 비슷합니다.   
> 아래의 코드는 FCFS 함수의 일부입니다.
```c
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
```
> 모든 스케줄링 함수는 위와 같은 반복문 내에서 스케줄링이 수행됩니다.   
> 스케줄링 함수 사이의 차이점은 대부분 프로세스를 레디 큐에서 가져와 수행하는 부분에서 발생합니다.
```c
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
```
> 위의 코드가 FCFS에서 프로세스가 수행되는 부분입니다.
### Round Robin
> 아래의 코드는 RR(time quantum=1)에서 프로세스가 수행되는 부분입니다.
```c
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
```
> FCFS에서 time quantum을 확인하는 조건문이 추가 되었습니다.
### Shortest Job First
> 아래는 SJF 함수의 코드입니다.
```c
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
```
> 프로세스 실행부는 FCFS와 동일하지만 프로세스를 실행하기 전에 레디 큐에 있는 프로세스들 중에서 가장 수행 시간이 짧은 프로세스 순으로 정렬하는 작업을 거칩니다.   
>    
> MLFQ 스케줄링 함수는 조금 다릅니다.   
>    
> 레디 큐를 multi level로 선언하여 각 레디 큐마다 우선 순위를 설정해야하기 때문입니다.
### Multi Level Feedback Queue
> 아래는 MLFQ 함수의 레디 큐 선언과 초기화 부분입니다.
```c
my_queue que[5]; // 레디 큐를 5개의 레벨로 구성, que[0] 부터 que[5] 까지 갈수록 우선순위가 낮아짐
for(int i=0; i<5; i++) {
	my_queue_init(&que[i]); // 레디 큐 초기화
}
```
> 이번 과제에서는 프로세스의 수가 5개이고 메모리 낭비 등을 고려하여 레디 큐를 5개의 레벨로 지정하였습니다.   
> 레디 큐를 배열로 선언하였기 때문에 레디 큐에서 프로세스를 삭제하고 수행하는 부분에서도 차이가 발생합니다.
```c
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
```
> 위의 코드는 레디 큐 배열에서 비어있지 않은 큐를 찾고 그 중에서 가장 우선 순위가 높은 큐에 존재하는 프로세스를 수행하기 위하여 삭제하는 부분입니다.   
>    
> 아래의 코드는 프로세스 수행부분 입니다.
```c
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
```
> time quantum을 확인하는 조건문과 프로세스의 우선 순위를 조정하는 조건문이 추가되었습니다.   
### 수행 결과
> 워크로드는 아래와 같습니다.
```c
my_proc proc[5] = { {'A', 3, 0, 0},
                    {'B', 6, 2, 0},
                    {'C', 4, 4, 0},
                    {'D', 5, 6, 0},
                    {'E', 2, 8, 0}}; // 프로세스 이름, 수행 시간, 도착 시간, 우선 순위 순
```
> 해당 워크로드에 대한 출력 결과입니다.   
>    
![result1](https://user-images.githubusercontent.com/39798011/123535600-cad8c180-d75f-11eb-93d9-0a3f42dd4457.png)

> 새로운 워크로드에 대한 출력 결과를 확인합니다.
```c
my_proc proc[5] = { {'A', 4, 0, 0},
                    {'B', 3, 1, 0},
                    {'C', 6, 2, 0},
                    {'D', 1, 4, 0},
                    {'E', 2, 9, 0}}; // 프로세스 이름, 수행 시간, 도착 시간, 우선 순위 순
```
> 해당 워크로드에 대한 출력 결과입니다.   
>    
![result2](https://user-images.githubusercontent.com/39798011/123535785-2ce5f680-d761-11eb-831f-e1ddcb466130.png)
