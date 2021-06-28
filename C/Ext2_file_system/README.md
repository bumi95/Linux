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
