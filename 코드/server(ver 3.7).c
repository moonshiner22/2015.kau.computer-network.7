/*
 * 2015 1학기 한국항공대학교 컴퓨터네트워크
 * 텀 프로젝트
 * 김 창섭, 김 경석, 서 동우, 조 동원
 * 채팅 프로그램
 */
//sendMessageToClient() 삭제
//서버가 LOGOUT("quit")을 보내면 모든 클라이언트들의 연결이 끊긴다.
//클라이언트가 볼 수 있는 모든 메시지는 서버도 볼 수 있다.(실제로 출력되도록 함수를 추가)
//매크로 RCVBUFSIZE 제거
//서버는 "//currentconnection"이라는 문자열을 입력하면, 현재 클라이언트들의 연결 수를 알 수 있다.
//클라이언트가 욕을 한 경우, 서버는 해당 내용이 완전히 보이고, 클라이언트에게는 '*'로 순화되어 전송된다.
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#define LOGOUT "quit"
#define CLTEND 0x05
#define OUTMSG " 님께서 퇴장하셨습니다."
#define SERVER_PORT 9999
#define OPTION1 "//currentconnection"   //서버를 위한 옵션 명령어

#define MAXPENDING 5   // 허용 대기 가능 인원.
#define MAXUSER 10     // 허용 접속 가능 인원.

//메시지는 언제나 발신자의 이름과 발신시간을 가지고 있다.
typedef struct _message {
	char m_userName[20];
	char m_buffer[100];
	char m_time[20];
} MESSAGE;

MESSAGE searchAndDeleteScolding(MESSAGE target);

int main()
{
	MESSAGE m_message;
  	fd_set status;
  
  	int servSock;   //서버 소켓번호
 	int clntSock[MAXUSER];   //클라이언트 소켓번호
  	int tempSock;
	int iMaxSock;
  
	unsigned int uiUser = 0;   //유저의 수

	int iCount;
	int i;
  
	struct sockaddr_in echoServAddr;   //서버 소켓
	struct sockaddr_in echoClntAddr;   //클라이언트 주소
  
	unsigned short echoServPort = SERVER_PORT;
	unsigned int clntLen; 

	time_t stTempTime;

  	int iRet;  //반환값 저장용 변수.

  	//unsigned char ucBuffer[500];  // 클라이이언트로부터의 메시지를 저장하는 버퍼.
	
 	servSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);  // Socket 생성.
	if(servSock < 0) {
		fprintf(stderr, "error: socket()\n");
		return 1;
	}

	memset(&echoServAddr, 0, sizeof(echoServAddr));
  	echoServAddr.sin_family = PF_INET;
  	echoServAddr.sin_addr.s_addr = htonl(INADDR_ANY);  // INADDR_ANY는 주소를 자동할당시켜 준다.
	echoServAddr.sin_port = htons(echoServPort);  // 포트 번호도 Big-Endian으로 바꿔주어야 한다. short형이므로 htons
  
	//생성된 소켓에 포트번호, IP등을 부여한다. (바인딩)
  	//bind함수에 쓰이는 인자는 구형 구조체이므로 캐스팅한다.
 	//블로킹 함수가 아니다.
	iRet = bind(servSock, (struct sockaddr *)&echoServAddr, sizeof(echoServAddr));
  	if(iRet < 0) {
  		close(servSock);   //소켓을 닫는다.
  		fprintf(stderr, "error: bind()\n");
  		return 1;
  	}

  	iRet = listen(servSock, MAXPENDING); // 최대 허용 대기 가능 인원.
	if(iRet < 0) {
  		close(servSock);  //소켓 종료
  		fprintf(stderr, "error: listen()\n");
  		return 1;
  	}

  	clntLen = sizeof(echoClntAddr);
   	iMaxSock = servSock + 1;  //셀렉트 문의 첫번째 인자는 무조건 제일 큰 소켓.
  
	printf("서버 프로그램을 시작합니다.\n");
  
	//본격적인 서버 프로그램의 구조가 시작됨.
  	while(1)
  	{
    	FD_ZERO(&status);   //변수 초기화(0으로)
    	FD_SET(0, &status);   //0번째 값을 1로 바꾼다(stdin) -> 키보드 감지. 0번째 파일, 즉 표준입력(키보드)의 반응을 받는다.
    	FD_SET(servSock, &status);   //랑데부 소켓(외부의 정보만을 받는 소켓)과 키보드만 감지한다.

		//현 시점에서 존재하는 클라이언트 소켓을 파악하고 select()의 감시대상이 되도록 세팅해준다.
   	 	for(i = 0; i < uiUser; i++) {
      		FD_SET(clntSock[i], &status);
			if(clntSock[i] >= iMaxSock) iMaxSock = clntSock[i] + 1;
		}
    
    	iRet = select(iMaxSock, &status, 0, 0, 0);
		if(iRet < 0) {   //셀렉트 함수가 실패할 경우
      		fprintf(stderr, "errer: select()\n");
      		strcpy(m_message.m_buffer, "server's select() is not activated, sorry, good bye");   //버퍼에 에러메시지 저장
			
			//접속한 클라이언트들에게 전송.
     		for(iCount = 0; iCount < uiUser; iCount++) write(clntSock[iCount], &m_message, sizeof(m_message));
			m_message.m_buffer[0] = CLTEND;
			for(iCount = 0; iCount < uiUser; iCount++) write(clntSock[iCount], &m_message, sizeof(m_message));
			
			break;
    	}
    
    	if(FD_ISSET(servSock, &status) == 1)   //새로운 클라이언트의 접속
    	{
			//들어오는 사람의 IP 및 정보가 요구되므로 2번째 인자가 클라이언트 정보.
		    //accept는 구 자료형 형식의 함수이기 때문에 강제 캐스팅.
		    //clntLen : 크기.
		    //accept에서는 접속이 될 때 까지 그대로 대기상태가 된다. 블로킹 함수.
		    //servSock : 랑데부 소켓(외부에서 받아들이기만 한다)
		    //clntSock : 커뮤니케이션 소켓. 내용을 처리할 때 쓰인다.
			tempSock = accept(servSock, (struct sockaddr *)&echoClntAddr, &clntLen);
		    if(tempSock < 0) {
				fprintf(stderr, "error: accept()\n");
				continue;
		    }
			
			printf("정체불명의 외부 클라이언트가 접속을 시도하였습니다.\n");
			printf("socket number: %d\n", tempSock);
			printf("외부 클라이언트의 IP: %s\n", inet_ntoa(echoClntAddr.sin_addr));   //Network to ASCII Code(수치로만 되어있는 IP주소를 사용자가 알아볼 수 있게 문자열로 바꾸어준다.)
			printf("외부 클라이언트의 PORT: %d\n", ntohs(echoClntAddr.sin_port));   // Big-Endian으로 저장된 클라이언트의 포트 정보를 서버에 맞게 Little-Endian으로 변환하여 출력한다.

      		if(uiUser >= MAXUSER) {   //사람이 다 찼는데 또 들어올 경우
				printf("최대 허용 가능 인원을 초과하여 외부 클라이언트의 접속을 거절하였습니다.\n");
				close(tempSock);
				continue;   //to while(1)
      		} else {
      			clntSock[uiUser] = tempSock;   //tempSock으로 받은 커뮤니케이션 소켓을 넣어준다.
      			uiUser += 1;   //접속 인원 + 1
				printf("신규 접속자 접속 완료\n");
     			printf("현재 접속자 수: %d명\n\n", uiUser);
			}
    	}
		else if(FD_ISSET(0, &status) == 1)   //서버가 메시지를 보내는 경우
    	{
			iRet = read(0, m_message.m_buffer, sizeof(m_message.m_buffer) - 1);   //키보드(0)로부터 데이터를 입력받음
			if(m_message.m_buffer[iRet - 1] == '\n') m_message.m_buffer[iRet - 1] = '\0';
		    m_message.m_buffer[iRet] = '\0';   //문자열의 완성을 위해 널문자를 추가
		    strcpy(m_message.m_userName, "SERVER");   //메시지의 근원인 이름을 "SERVER"로 채워준다.
			//시간 역시 MESSAGE structure에 채워준다.
		    time(&stTempTime);
		    strftime(m_message.m_time, 26, "%Y-%m-%d %H:%M:%S", localtime(&stTempTime));
		      
			//서버가 LOGOUT("quit")을 입력하면 클라이언트들의 연결을 끊어낸다.
			//이후 서버 자신도 실행을 종료한다.
			//일반적인 메시지라면 모든 클라이언트에게 메시지를 보낸다.
			//옵션 명령어도 처리한다.
			if(strcmp(LOGOUT, m_message.m_buffer) == 0) {
				strcpy(m_message.m_buffer, "sorry!, server is shut now, good bye");
				for(iCount = 0; iCount < uiUser; iCount++) write(clntSock[iCount], &m_message, sizeof(m_message));
				
				m_message.m_buffer[0] = CLTEND;
				for(iCount = 0; iCount < uiUser; iCount++) write(clntSock[iCount], &m_message, sizeof(m_message));
				
				break;
			} else if(strcmp(OPTION1, m_message.m_buffer) == 0) {
				printf("현재 접속자 수: %d명\n", uiUser);
			} else {
				printf("from %s[%s] at (%s): %s\n", m_message.m_userName, inet_ntoa(echoServAddr.sin_addr), m_message.m_time, m_message.m_buffer);
				for(iCount = 0; iCount < uiUser; iCount++) write(clntSock[iCount], &m_message, sizeof(m_message));
			}
    	}
		else   //그 외의 일반적인 경우의 작동
   		{
      		for(iCount = 0; iCount < uiUser; iCount++)   //모든 클라이언트를 훑으면서 다음 if문을 실행한다.
      		{
				//클라이언트의 변화를 감지할 경우
        		if(FD_ISSET(clntSock[iCount], &status) == 1)
        		{
					//커뮤니케이션 소켓으로부터 값을 읽어온다.
				  	//read함수는 글자 수를 반환하므로 iRet에 글자수를 임시로 저장한다.
				  	iRet = read(clntSock[iCount], &m_message, sizeof(m_message));
				  	//m_message.m_buffer[iRet] = '\0';   //마지막 글자는 \n(개행문자)이므로 NULL로 처리한다.

          			if(iRet)   //클라이언트에서 무언가를 읽음
					{
						//메시지가 LOGOUT(이 경우 "quit")인 경우
					  	if(strcmp(LOGOUT, m_message.m_buffer) == 0)
					   	{
							//CLTEND를 해당 클라이언트에게 보낸다.
						    m_message.m_buffer[0] = CLTEND;
						    write(clntSock[iCount], &m_message, sizeof(m_message));
						      
							//해당 통신 소켓을 닫는다.
						    clntSock[iCount] = clntSock[uiUser - 1];
						    close(clntSock[iCount]);
						      
						    strcpy(m_message.m_buffer, m_message.m_userName);
						    strcat(m_message.m_buffer, OUTMSG);
							printf("%s\n", m_message.m_buffer);
							//클라이언트들에게도 외부인의 종료를 알린다.
						    for(i = 0; i < uiUser; i++) write(clntSock[i], &m_message, sizeof(m_message)); 

						    uiUser -= 1;

						    continue;
						}
						//메시지가 LOGOUT이 아닌 일반적인 경우
						else
						{
							//클라이언트의 메시지는 서버에서도 출력한다.
							//fflush(stdout);
						    printf("from %s[%s] at (%s): %s\n", m_message.m_userName, inet_ntoa(echoClntAddr.sin_addr), m_message.m_time, m_message.m_buffer);
						    
							m_message = searchAndDeleteScolding(m_message);
							
							//모든 클라이언트들에게 전송
						    for(i = 0; i < uiUser; i++) write(clntSock[i], &m_message, sizeof(m_message));
						}
          			}//if(iRet)
        		}//if(FD_ISSET(clntSock[iCount], &status) == 1)
      		}//for(iCount = 0; iCount < uiUser; iCount++)
    	}//else
	}//while(1)

	close(servSock);
	for(i = 0; i < uiUser; i++) close(clntSock[i]);
	printf("서버를 종료합니다.\n");
  	return 0;
}

MESSAGE searchAndDeleteScolding(MESSAGE target)
{
	char * scoldings[] = {"shit", "fuck", "바보", "멍청이"};
	int i;
	int j;
	char * temp;
	MESSAGE return_value;
	
	for(i = 0; i < sizeof(scoldings) / sizeof(scoldings[0]); i++)
	{
		while(temp = strstr(target.m_buffer, scoldings[i])) {
			for(j = 0; j < strlen(scoldings[i]); j++) {
				*temp = '*';
				temp++;
			}
		}
	}

	return_value = target;
	return return_value;
}