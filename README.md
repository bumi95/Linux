[졸음 운전 방지 시스템 개발 프로젝트](https://github.com/bumi95/Linux/tree/main/Safe_driving_project)
====================================
## 개발 환경 및 요구 조건
> 개발 언어
* Python
> 개발 환경
* Jetson nano, Ubuntu 18.04 LTS
> 필수 라이브러리
* OpenCV, Dlib, face_recognition
> 요구 조건
* Jetson nano kit, CSI camera module, sensor
## 시스템 블록 다이어그램
![block](https://user-images.githubusercontent.com/39798011/123509922-521c2b80-d6b3-11eb-8efe-4097f55a11e9.jpg)

## 수행 내용
> 저는 얼굴 인식 코드 분석과 성능 개선 부분을 담당하였습니다.   
> Jetson nano 보드에 연결한 카메라 모듈의 입력 값을 gstreamer_pipeline을 이용하여 전달 받습니다.   
> opencv 라이브러리의 videocapture 함수를 이용하여 gstreamer_pipeline으로 설정한 카메라를 실행 시킵니다.
```python
def gstreamer_pipeline( # 해상도와 영상의 크기, 프레임 등 설정
    capture_width=3280,
    capture_height=2464,
    display_width=960,
    display_height=720,
    framerate=21,
    flip_method=0,
    ):
    return (
        "nvarguscamerasrc ! "
        "video/x-raw(memory:NVMM), "
        "width=(int)%d, height=(int)%d, "
        "format=(string)NV12, framerate=(fraction)%d/1 ! "
        "nvvidconv flip-method=%d ! "
        "video/x-raw, width=(int)%d, height=(int)%d, format=(string)BGRx ! "
        "videoconvert ! "
        "video/x-raw, format=(string)BGR ! appsink"
        % (
            capture_width,
            capture_height,
            framerate,
            flip_method,
            display_width,
            display_height,
        )
    )
video_capture = cv2.VideoCapture(gstreamer_pipeline(flip_method=0), cv2.CAP_GSTREAMER) # 영상 출력
```
> 실행된 카메라 영상 정보(frame)를 read를 통하여 읽어옵니다.   
> 얼굴 인식 속도 개선을 위하여 opencv의 resize 함수를 이용하여 이미지의 크기를 1/4로 줄입니다.   
> 읽어온 이미지는 RGB 값이 아닌 BGR 값으로 색상이 반전되어 있으므로 RGB 값으로 반전시킵니다.
```python
ret, frame = video_capture.read()
small_frame = cv2.resize(frame, (0, 0), fx=0.25, fy=0.25) # 크기 조정 -> 속도 개선
rgb_small_frame = small_frame[:, :, ::-1] # RGB 값으로 변환
```
> face_recognition 라이브러리의 face_location, face_encodings 함수를 이용하여 매개 변수로 받은 이미지에서 얼굴을 찾고 얼굴 값을 인코딩하여 저장합니다.   
>    
> 매개 변수로 받는 이미지는 1/4로 크기를 줄인 이미지로 원래 크기의 이미지에서 얼굴을 찾는 속도보다 더 빠른 속도로 찾을 수 있습니다.   
>    
> 얼굴 인코딩 값을 compare_faces 함수를 이용하여 개인의 얼굴 사진을 이용한 데이터와 비교합니다.   
> 비교를 통하여 안전한 상태라면 safe, 안전하지 않은 상태라면 dangerous를 face_name 리스트에 추가합니다.   
>    
> 얼굴을 인식하는 부분에서 많은 시간이 소요되므로 이미지 프레임을 4번 보낼 때 1번 꼴로 수행되도록 하여 속도를 개선하였습니다.
```python
if process_this_frame == 3: # 4번에 1번 꼴로 수행
  face_locations = face_recognition.face_locations(rgb_small_frame)
  face_encodings = face_recognition.face_encodings(rgb_small_frame, face_locations)
  process_this_frame = 0

  face_names = []
  for face_encoding in face_encodings:
    matches = face_recognition.compare_faces(known_face_encodings, face_encoding)
    name = "dangerous"
    
    face_distances = face_recognition.face_distance(known_face_encodings, face_encoding)
    best_match_index = np.argmin(face_distances)
    if matches[best_match_index]:
      name = known_face_names[best_match_index]
    
    face_names.append(name)
    
process_this_frame+=1
```
> 실제 카메라 영상에 출력할 때는 이미지를 원래의 크기로 되돌려 놓습니다.   
> 또한 얼굴 부분에 안전한 상태인지 아닌지 사각형 박스와 face_name 리스트의 값을 함께 출력합니다.   
> 안전한 상태라면 초록색, 아니라면 빨간색으로 출력합니다.
```python
for (top, right, bottom, left), name in zip(face_locations, face_names):
  top *= 4
  right *= 4
  bottom *= 4
  left *= 4

  if name == "safe" :
    cv2.rectangle(frame, (left, top), (right, bottom), (0, 255, 0), 2)
    cv2.rectangle(frame, (left, bottom - 35), (right, bottom), (0, 255, 0), cv2.FILLED)
        
  else :
    cv2.rectangle(frame, (left, top), (right, bottom), (0, 0, 255), 2)
    cv2.rectangle(frame, (left, bottom - 35), (right, bottom), (0, 0, 255), cv2.FILLED)
    
  font = cv2.FONT_HERSHEY_DUPLEX
  cv2.putText(frame, name, (left + 6, bottom - 6), font, 1.0, (255, 255, 255), 1)
```
## 제한 사항
> 개발 제한 사항
* 다양한 얼굴 데이터 셋을 만들고 적용하지 못해 개인의 얼굴 사진을 이용
* 여러 센서를 구비하지 못하여 경고음을 울리거나 진동이 울리는 알람을 구현하지 못함
* * *
[프로세스 스케줄링 시뮬레이터 구현](https://github.com/bumi95/Linux/tree/main/C/process_scheduling)
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
### 
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
* * *
[생산자 소비자 문제 구현](https://github.com/bumi95/Linux/tree/main/C/prod_cons_problem)
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

'''

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
* * *
Ext2 파일시스템 내부구조 분석
===========================
## 문제 정의
> ramdisk를 이용하여 Ext2 file system 환경을 구축하고 구조를 분석하여 자신의 학번 끝 세 자리에 해당하는 파일을 찾고, 그 파일에 속한 4개의 블록을 찾습니다. (학번 : 32141925)
## 분석 내용
> xxd 명령어를 이용하여 superblock, descriptor, inode block과 data block의 내용을 분석하여 파일을 찾습니다.
* 파일을 찾는 방법
	+ superblock의 내용을 분석하여 inode의 개수, 블록 그룹 당 inode의 개수, 데이터 블록의 개수, 블록 그룹 당 데이터 블록의 개수, 데이터 블록의 크기와 블록 그룹 번호를 파악합니다.
	+ descriptor의 내용을 분석하여 inode table의 시작 블록 번호를 파악하고 root inode 번호인 2번을 이용하여 root 디렉터리 inode의 위치를 파악합니다.
	+ root 디렉터리 inode의 내용을 분석하여 root 디렉터리가 존재하는 블록의 위치를 파악합니다.
	+ root 디렉터리 데이터 블록의 내용을 분석하여 9번 디렉터리의 inode 번호를 파악합니다.
	+ 찾은 inode 번호를 이용하여 어떤 블록 그룹에 속해있고 inode table의 몇 번째에 위치하는지 파악합니다.
	+ 해당 블록 그룹의 inode table을 분석하여 9번 디렉터리의 inode의 위치를 찾고 inode에서 디렉터리가 존재하는 블록의 위치를 파악합니다.
	+ 9번 디렉터리 데이터 블록의 내용을 분석하여 25번 파일의 inode 번호를 찾아 어떤 블록 그룹에 속해있고 inode table의 몇 번째에 위치하는지 파악합니다.
	+ 25번 파일의 inode 번호를 이용해 inode 위치를 찾고 inode의 direct block pointer와 indirect block pointer를 분석하여 파일이 위치하는 블록을 찾고 내용을 출력합니다.   
> ramdisk 모듈을 적재하고 마운트한 후 파일들을 생성하기 위하여 미리 작성된 스크립트를 실행합니다.   
> 스크립트 실행 후에 0~9번 디렉터리가 생성된 것을 확인하고 찾아야 할 파일에 한 블록을 추가합니다.(총 4개의 블록을 찾아야 함)   
```
./apd mnt/9/25 13 9/25-13 // apd는 조교가 제공하는 도구
```
> Ext2 의 정보는 리틀 엔디안으로 저장되어있어 메타 데이터를 읽을 때는 보여지는 데이터를 바이트별로 반대로 읽어야 합니다.   
> super user 모드에서 분석을 수행합니다.   
>    
![1](https://user-images.githubusercontent.com/39798011/123539950-442fde80-d777-11eb-8bea-f3f807cf514c.png)
> 블록 그룹 0의 superblock은 부트섹터 다음에 위치하므로 1024B에 위치합니다.   
> 위 그림은 해당 위치에서 256B만큼 데이터를 출력한 내용입니다.
* inode의 수 : 0x8000
* block의 수 : 0x20000
* 데이터 블록의 크기 : 0x2(4KB)
* 그룹 당 inode의 수 : 0x2000
* 그룹 당 block의 수 : 0x8000
* 블록 그룹 번호 : 0x0
> 위 그림에서 위와 같은 정보를 얻을 수 있습니다.   
> 총 블록 수와 그룹 당 블록 수를 확인했을 때 블록 그룹은 총 4개로 이루어져 있다고 유추할 수 있습니다.   
>    
![2](https://user-images.githubusercontent.com/39798011/123540038-de902200-d777-11eb-83c6-2e88f71c5976.png)
> superblock의 다음 블록에 위치한 group descriptor table 영역은 block bitmap, inode bitmap, inode table의 시작 위치를 알 수 있습니다.   
> 위 그림에서 inode table의 시작 위치는 0x23 블록 임을 알 수 있습니다.(단위 4KB)   
> 또한, root 디렉터리의 inode 번호는 2번이므로 위치 계산식을 이용하여 0번 블록 그룹의 inode table 1번 위치에 있는 것을 알 수 있습니다.   
> inode의 크기는 256B이므로 inode table의 1번 위치인 0x23100을 확인하면 아래 그림과 같습니다.   
>    
![3](https://user-images.githubusercontent.com/39798011/123540127-59593d00-d778-11eb-9d12-98c68341a1bf.png)
> 0x41ed는 파일의 mode를 나타내고 drwxr-xr-x이므로 디렉터리임을 알 수 있습니다.   
> 즉, 이 inode가 가지고 있는 정보는 root 디렉터리임을 의미합니다.   
> inode의 첫 번째 direct block pointer의 위치인 0x223 블록을 따라가면 root 디렉터리 블록으로 갈 수 있습니다.   
>    
![4](https://user-images.githubusercontent.com/39798011/123540382-9ffb6700-d779-11eb-9bd1-c0c3d8e64350.png)
> 찾아야 하는 디렉터리는 9번 디렉터리 입니다.   
> 디렉터리의 구조에 따라 inode의 번호, 파일의 type과 이름을 알 수 있습니다.   
> 0x2는 디렉터리를 의미하고 0x60cb가 9번 디렉터리의 inode 번호입니다.   
>    
> 위치 계산식을 이용하여 9번 디렉터리 inode가 블록 그룹 3의 inode table 202번 위치에 존재하는 것을 알 수 있습니다.   
> 블록 그룹은 0~3 까지 존재하고 각 블록 그룹의 시작 블록은 0x0, 0x8000, 0x10000, 0x18000 입니다.   
>    
> 따라서, 블록 그룹 3의 시작 블록은 0x18000 블록입니다.   
>    
![5](https://user-images.githubusercontent.com/39798011/123540967-adfeb700-d77c-11eb-8b2a-5f55a8d02baa.png)
> 위 그림처럼 0x18000 블록 위치에 블록 그룹 3의 superblock이 위치하는 것을 확인할 수 있습니다.   
> descriptor를 통하여 inode table 위치를 확인하면 아래와 같습니다.   
>    
![6](https://user-images.githubusercontent.com/39798011/123540989-dc7c9200-d77c-11eb-9c44-3f9a2c4f776b.png)
> descriptor의 위치는 블록 그룹 0과 마찬가지로 superblock의 다음 블록에 위치합니다.   
> 따라서 superblock 위치에서 4KB를 더한 위치입니다.   
> inode table의 시작 위치는 블록 그룹 0과 똑같이 0x23번째 블록인 것을 알 수 있습니다.   
> 0x23번째 블록이 inode table의 첫 번째 위치이고 9번 디렉터리의 inode는 202번째 위치에 존재하므로 블록의 위치를 10진수로 변환하여 더한 후 다시 16진수로 변환하면 0x1802fa00 위치에 존재한다는 것을 알 수 있습니다.   
>    
![7](https://user-images.githubusercontent.com/39798011/123541271-99232300-d77e-11eb-9323-27fa1ca8e31e.png)
> 위 그림은 9번 디렉터리 inode가 존재하는 위치입니다.   
> 마찬가지로 0x41ed = drwxr-xr-x 모드이며 9번 디렉터리의 위치는 0x1020b블록 즉, 0x1020b000 위치입니다.   
>    
![8](https://user-images.githubusercontent.com/39798011/123541320-d687b080-d77e-11eb-833f-3893c9146bc0.png)
> 위 그림은 9번 디렉터리의 내용입니다.   
> 25번 파일의 이름과 파일 type, 파일 inode 번호를 알 수 있습니다.   
> inode 번호는 0x60e5로 위치 계산식을 통하여 블록 그룹 3의 inode table 228번째 위치하는 것을 확인할 수 있습니다.   
> 따라서, 25번 파일의 inode의 위치는 0x18031400 임을 알 수 있습니다.   
>    
![9](https://user-images.githubusercontent.com/39798011/123541365-1e0e3c80-d77f-11eb-9057-58fd0a039cb7.png)
> 위 그림은 25번 파일의 inode 입니다.   
> mode가 0x81a4로 8진수로 100644 입니다.   
> 100은 일반 파일임을 나타내고 644가 접근 권한 rw-r--r-- 을 나타냅니다.   
> 해당 파일은 총 4개의 블록으로 이루어져 있습니다.   
> direct block pointer가 3개의 블록을 가리키고 있고, 나머지 추가한 하나의 블록을 indirect block pointer가 가리키고 있습니다.   
> 먼저 direct block pointer 3개의 위치를 찾아가면 아래와 같은 파일의 내용을 얻을 수 있습니다.   
>    
![10](https://user-images.githubusercontent.com/39798011/123541418-79402f00-d77f-11eb-8e84-8b9d800f0c66.png)
![11](https://user-images.githubusercontent.com/39798011/123541489-d3d98b00-d77f-11eb-9d86-ad4b2755eb6e.png)
> 9/25-1 ~ 3 까지 direct block pointer를 이용하여 찾았고 나머지 9/25-13은 indirect block pointer를 따라가 데이터 블록을 가리키는 포인터의 위치를 찾아야 합니다.   
>    
![12](https://user-images.githubusercontent.com/39798011/123541552-2155f800-d780-11eb-88dc-fe3332dfbb99.png)
> indirect pointer를 따라가면 9/25-13 데이터 블록을 가리키는 포인터를 찾을 수 있습니다.   
>    
![13](https://user-images.githubusercontent.com/39798011/123541597-52362d00-d780-11eb-8a3b-d4c96cf0d1e5.png)
