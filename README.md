# SMA_raspberry


## header files
--- 
### data.h : data struct 및 mode 정의

- data struct 의 경우 string(DC part)이 나누어져 있는 것을 모두 가져와서 합을 구합니다

- mode의 경우 WCDMA, LAN, LTE 모드로 나눠져 있으며 LAN의 경우 이더넷 통신이 인터넷에 연결되어 있어야 합니다
---
### tcpclient.h : TCP(LAN) 통신을 위한 클래스 정의

---

### mainwindow.h : 타이머 및 슬롯, 클릭이벤트, 아이피 정의

- Timer(타이머) : 시간관련 설정
-- invTimer : 10초마다 인버터 데이터 요청
-- serTimer : 서버 전송 주기(인버터 갯수에따라 5~10분)
-- cheTimer : 1초마다 시간 체크 및 정전 상태 확인
-- StrTimer : 5초간 설정 시간
-- stpTimer : 정지 버튼 눌렀을 시 10분 후 스스로 동작 되도록 타이머 설정


- slot(슬롯) : 타임 아웃 및 메뉴 설정시 동작하는 함수 정의

- clickEvent(slot) : 버튼 클릭시 동작 함수 정의 

- QFutureWatcher : Thread 완료시 동작

- invIP : 인버터별 아이피 설정(기본 192.168.0.101~120)
- 그 외 인버터 용량별 함수 정의

---

## CPP files
---
### clickevent.cpp : 클릭이벤트 정의(slot)
- on_pushButton_clicked(발전소 번호 설정) : 파일저장
- on_pushButton_2_clicked(인버터 갯수 설정) : 파일저장 및 서버 전송 시간 설정 - 7개기준
- on_pushButton_3_clicked(시작버튼) : invTimer,serTimer,cheTimer 동작
- on_pushButton_4_clicked(정지버튼) : invTimer,serTimer,cheTimer 정지, stpTimer 동작
- on_pushButton_5_clicked(인버터 요청) : Timer없이 인버터 요청
- on_pushButton_6_clicked(서버 전송) : Timer없이 서버 전송, 전송 중일 때 동작 하지 않음
- on_pushButton_7_clicked(로그 보기) : 에러 로그 저장한 파일 실행
- on_pushButton_8_clicked(CSQ 요청) : CSQ값을 확인
- on_pushButton_9_clicked(모드 교체) : WCDMA->LAN->LTE 순으로 교체 및 파일 저장

- hw_reset : GPIO를 통한 보드쪽 리셋 -> digitalWrite(0,(1)); //외부 버튼을 이용한 리셋 기능
- sw_reset : command를 통한 리셋(reboot)

---
### csq.cpp : csq 관련 함수 정의

 - csq_ok : csq에 대한 응답이 있을시 CSQ 수치를 가져옴
 - req_csq : csq를 요청하는 함수
 
---
### file.cpp
 - getFileNum(번호 받기) : 특정 파일 이름을 통해 파일 내용을 리턴
 - setFileNUm(번호 입력) : 특정 파일에 데이터를 입력
 - setFileLog(로그 입력) : 로그를 로그파일에 입력(삭제 예정)
---
### lan.cpp
- addPacket : checksum계산을 위해 사용
- plantNumber에 따른 eeport 번호 분류 7000~7500 : 7777, 7500~8000 : 7778.. (500마다 1씩 증가)
- SendMessageHstec : client 연결 결과에 따른 결과 반환
---
### lte.cpp
- SendLTE : plantNumber에 따른 eeport 번호 분류 7000~7500 : 7777, 7500~8000 : 7778.. (500마다 1씩 증가)
- 시퀀스
- ATE : echo가 되지 않도록 설정 
- CSQ : csq 수치를 요청함
- STAT : 현재 통신망과 연결 상태 확인
- RNDISDATA : 데이터 모드 ON/OFF
- WSOCR : 소켓에 주소와 포트 설정
- WSOCO : 소켓 Open 설정
- WSOWR : 데이터 전송 내용 설정
- WSOCL : 소켓 close 설정

- luart_ch : system("sudo chmod 777 /dev/ttyAMA0") 명령어를 통해 uart로 전송
- lsend_append : 전송 데이터 바인딩
- lsend_ok : 전송 유무 확인

---
### wcdma.cpp
- SendWCDMA : plantNumber에 따른 eeport 번호 분류 7000~7500 : 7777, 7500~8000 : 7778.. (500마다 1씩 증가)
- 시퀀스
- ATE : echo가 되지 않도록 설정 
- CSQ : csq 수치를 요청함
- TCPTYPE : TCP mode 설정
- PPPOPEN : 소켓오픈
- TCPOPEN : 주소와 포트번호 설정
- TCPWRITE : 서버로 데이터 전송
- TCPREAD : 서버로부터 데이터 수신
- TCPCLOSE : 설정한 것을 오프
- PPPCLOSE : 소켓 오프

- uart_ch : system("sudo chmod 777 /dev/ttyAMA0") 명령어를 통해 uart로 전송
- send_append : 전송 데이터 바인딩
- send_ok : 전송 유무 확인
---
### tcpclient.cpp
- TcpClient : 생성자, 버퍼 초기화, 소켓 생성
- setIpAddress : 주소 설정
- TcpConnect : IP 주소와 포트 바인드, 5초 동안 응답 없을시 false 리턴(인버터 통신)
- connectToServer : 서버쪽으로 통신
- TcpDisconnect : 연결 해제
- onConnectServer : 연결 상태 확인
- sendRequst : 목적지로 데이터를 보냄
- readMessage : 목적지로부터 데이터를 읽어 들임
- getBuf : 버퍼를 읽어 들임
- setFileLog : 에러 로그 저장(삭제 예정)
---
### main.cpp
- 윈도우를 띄우는 부분
---
### mainwindow.cpp
- MainWindow : 초기 설정(생성자), Timer interval 설정, watcher binding, GPIO 설정
- invslot : 인버터 용량별 요청
- SendMessage : 용량별 요청에 대한 결과 반환
- on_comboBox_activated : 콤보박스를 통한 용량 설정
- cheslot : 1초마다 필요한 함수 호출(정전 확인)
