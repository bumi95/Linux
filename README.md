졸음 운전 방지 시스템 개발 프로젝트
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





