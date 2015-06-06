//atoi() 함수 삭제
//SendStringToClient() 함수 -> sendMessaegToClient() 변경

//error message 출력 스트림을 stdout에서 stderr로 바꿈, 지난 번 코드에서 실수하였음
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#define RCVBUFSIZE 32
#define ENDMSG "@ENDM"
#define LOGOUT "quit"
#define CLTEND 0x05
#define OUTMSG " 님께서 퇴장하셨습니다."

#define MAXPENDING 5   // 최대 허용 대기 가능 인원.
#define MAXUSER 10     // 최대 허용 접속 가능 인원.

typedef struct _message {
	char m_userName[20];
	char m_buffer[100];
	char m_time[20];
} MESSAGE;

void sendMessageToClient(int s, const char * msg);

int main()
{
	MESSAGE m_message;
  
  	fd_set status;
  
  	int servSock;   //서버소켓번호
 	int clntSock[MAXUSER];   // 클라이언트 소켓
  	int tempSock;
	int iMaxSock;
  
	unsigned int uiUser;

	int iCount;
	int i;
  
	struct sockaddr_in echoServAddr;  //서버 소켓
	struct sockaddr_in echoClntAddr;  //클라이언트 주소
  
	unsigned short echoServPort = 9999;  // 서버 포트 9999
	unsigned int clntLen; 

	time_t stTempTime;

  	int iRet;  //반환값 저장 변수.

  	unsigned char ucBuffer[500];  // 클라이이언트로부터의 메시지를 저장하는 버퍼.

 	servSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);  // Socket 생성.
	if(servSock < 0) {
		fprintf(stderr, "error: socket()\n");
    		return 1;
	}

	memset(&echoServAddr, 0, sizeof(echoServAddr));
  	echoServAddr.sin_family = PF_INET;
  	echoServAddr.sin_addr.s_addr = htonl(INADDR_ANY);  // INADDR_ANY는 주소를 자동할당시켜 준다.
	echoServAddr.sin_port = htons(echoServPort);  // 포트 번호도 Big-Endian으로 바꿔주어야 한다. short형이므로 htons
  
	iRet = bind(servSock, (struct sockaddr *)&echoServAddr, sizeof(echoServAddr));
  	// 생성된 소켓에 포트 번호, IP등을 부여한다. 바인딩
  	// bind함수에 쓰이는 인자는 구형 구조체이므로 캐스팅한다.
 	// 블로킹 함수가 아니다.
  	if(iRet < 0) {
  		close(servSock);
  		fprintf(stderr, "error: bind()\n" );
  		return 1;
  	}

  	iRet = listen(servSock, MAXPENDING); // 최대 허용 대기 가능 인
	if(iRet < 0) {  
  		close(servSock);  //소켓 종료
  		fprintf(stderr, "error: listen()\n" );
  		return 1;
  	}

  	clntLen = sizeof(echoClntAddr);
   	iMaxSock = servSock + 1;  //셀렉트 문의 첫번째 인자는 무조건 제일 큰 소켓.
  	uiUser = 0;
  
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
		if(iRet < 0) {		//셀렉트 함수가 실패할 경우 
      			fprintf(stderr, "errer: select()\n" );
      			strcpy(m_message.m_buffer, "error: 우server is destroyed, sorry, good bye");		//버퍼에 에러메시지 저장
      
			// 접속한 클라이언트들에게 전송.
     			for(iCount = 0; iCount < uiUser; iCount++) write(clntSock[iCount], &m_message, sizeof(m_message));

			//클라이언트에게 "/q"를 보내는 부분이었음

			// 접속한 클라이언트들에게 전송.
      			for(iCount = 0; iCount < uiUser; iCount++) write(clntSock[iCount], &m_message, sizeof(m_message));

			break;
    		}
    
    		if(FD_ISSET(servSock, &status) == 1)   //새로운 클라이언트의 접속
    		{
			tempSock = accept(servSock, (struct sockaddr *)&echoClntAddr, &clntLen);
		      	// 들어오는 사람의 IP 및 정보가 요구되므로 2번째 인자가 클라이언트 정보.
		     	// accept는 구 자료형 형식의 함수이기 때문에 강제 캐스팅
		      	// clntLen : 크기.
		      	// accept에서는 접속이 될 때 까지 그대로 대기상태가 된다. 블로킹 함수.
		      	// servSock : 랑데부 소켓(외부에서 받아들이기만 한다)
		      	// clntSock : 커뮤니케이션 소켓. 내용을 처리할 때 쓰인다.
  
		      	printf("socket number : %d\n", tempSock);
		     	if(tempSock < 0) {
				fprintf(stderr, "error: accept()\n");
				continue;
		      	}
    
      			// Network to ASCII Code(수치로만 되어있는 IP주소를 사용자가 알아볼 수 있게 문자열로 바꾸어준다.)
			printf("클라이언트 접속 IP : %s\n", inet_ntoa(echoClntAddr.sin_addr));
		      	// Big-Endian으로 저장된 클라이언트의 포트 정보를 서버에 맞게 Little-Endian으로 변환하여 출력한다.
			printf("클라이언트 접속 PORT : %d\n", ntohs(echoClntAddr.sin_port));

      			if(uiUser >= MAXUSER) {   // 사람이 다 찼는데 또 들어올 경우
        			close(tempSock);
        			continue;   //to while(1)
      			} else {
      				clntSock[uiUser] = tempSock;  // tempSock으로 받은 커뮤니케이션 소켓을 넣어준다.
      				uiUser += 1;  // 접속 인원 + 1
     				printf("현재 접속자 수: %d명\n", uiUser);
			}
    		}
		else if(FD_ISSET(0, &status) == 1)   //키보드의 반응을 보고 서버가 메시지를 보냄
    		{
			iRet = read(0, m_message.m_buffer, sizeof(m_message.m_buffer));   //키보드(0)로부터 데이터를 입력받음
		      	m_message.m_buffer[iRet - 1] = 0;   //문자열의 완성을 위해 널문자를 추가
		      	strcpy(m_message.m_userName, "Server");   //메시지의 근원인 이름을 "Server"로 채워준다.

			//시간 역시 MESSAGE structure에 채워준다.
		      	time(&stTempTime);
		      	strftime(m_message.m_time, 26, "%Y-%m-%d %H:%M:%S", localtime(&stTempTime));
		      
		      	if(m_message.m_buffer[0] == CLTEND) {
				strcpy(m_message.m_buffer, ENDMSG);
				//클라이언트들에게 전송
				for(iCount = 0; iCount < uiUser; iCount++) write(clntSock[iCount], &m_message, sizeof(m_message));
				break;
		      	} else {
				//클라이언트들에게 전송
		     		for(iCount = 0; iCount < uiUser; iCount++) write(clntSock[iCount], &m_message, sizeof(m_message));
			}
    		}
		else   //그 외의 일반적인 경우의 작동
   		{
      			for(iCount = 0; iCount < uiUser; iCount++)   //모든 클라이언트를 훑으면서 다음 if문을 실행한다.
      			{
        			if(FD_ISSET(clntSock[iCount], &status) == 1)  //클라이언트의 변화를 감지할 경우
        			{
				  	iRet = read(clntSock[iCount], &m_message, sizeof(ucBuffer) - 1);
				  	// 커뮤니케이션 소켓으로부터 값을 읽어온다.
				  	// read함수는 글자 수를 반환하므로 iRet에 글자수를 임시로 저장한다.
			      
				  	ucBuffer[ iRet ] = 0;  // 마지막 글자는 \n(개행문자)이므로 NULL로 처리한다.

          				if(iRet)   //클라이언트에서 무언가를 읽음
					{
						//메시지가 LOGOUT(이 경우 "quit")인 경우
					  	if(strcmp(LOGOUT, m_message.m_buffer) == 0)
					   	{
							//CLTEND를 해당 클라이언트에게 보낸다.
						      	m_message.m_buffer[0] = CLTEND;
						     	write(clntSock[iCount], &m_message, sizeof(m_message));
						      
							//해당 통신 소켓을 닫는다.
						      	clntSock[iCount] = clntSock[uiUser-1];
						      	close(clntSock[iCount]);
						      
						      	strcpy(m_message.m_buffer, m_message.m_userName);
						      	//printf( "%s\n", m_message.m_buffer );

						      	strcat(m_message.m_buffer, ENDMSG);
						      	//printf( "%s\n", m_message.m_buffer );
              
							//클라이언트들에게 외부인의 종료를 알린다.
						      	for(i = 0; i < uiUser; i++) write(clntSock[i], &m_message, sizeof(m_message)); 

						      	uiUser -= 1;

						      	continue;
            					  }
						  //메시지가 LOGOUT이 아닌 일반적인 경우
						  else
						  {
						    	//printf("from %s [%s]: ", m_message.m_userName, inet_ntoa(echoClntAddr.sin_addr));
						      	
							//fflush(stdout);
						      	//printf("%s", m_message.m_buffer);  // 버퍼에 있는 내용을 stdout(화면)에 출력한다.
						      	//printf("(%s)]\n", m_message.m_time );
						      	//sendMessaegToClient( clntSock, inet_ntoa( echoClntAddr.sin_addr) );
						      	//sendMessageToclient( clntSock, "님의 말 : " );
						      
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
  
  	return 0;
}

void sendMessageToClient(int s, const char * msg)
{
	write(s, msg, strlen(msg));
}
