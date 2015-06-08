/*
 * 2015 1학기 한국항공대학교 컴퓨터네트워크
 * 텀 프로젝트
 * 김 창섭, 김 경석, 서 동우, 조 동원
 * 채팅 프로그램
 */
 //프로그램 시작시 서버의 IP를 잘못 입력한 경우, 에러메시지를 출력할 수도 있다.
 //사용자는 자신의 이름을 입력할 때 중간에 공백이 있으면 안 된다. 또한 "SERVER"라는 이름은 사용 불가능하다.
 //사용자가 MESSAGE structure의 메시지 버퍼를 넘어가는 길이의 메시지를 입력할 경우, read()시 99바이트와 1바이트의 NULL문자를 입력하므로 총 100바이트의 메시지 전송을 완료하고, 아직 남아있는 메시지에 대해 클라이언트 프로그램은 사용자가 또 다시 메시지를 입력한 것으로 판단한다.
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

#define MAXPENDING 5   // 최대 허용 대기 가능 인원.
#define MAXUSER 10   // 최대 허용 접속 가능 인원.

typedef struct _message {
	char m_userName[20];
	char m_buffer[100];
	char m_time[20];
} MESSAGE;

int main(int argc, char * * argv)
{
	fd_set status;
  	MESSAGE m_message;

	char m_userName[20];
  
  	int sock;

  	struct sockaddr_in echoServAddr;  //서버 구조체
  	unsigned short echoServPort;  // 서버 포트
  	char * servIP = *(argv+1);  //서버 주소
  	char * echoString;
  	char buffer[500];  // 버퍼

  	time_t stTempTime;
  	int iRet;

	if(argc != 3) {
		printf("[Usage]: ./prog IP name(no space!)\n");
		return 1;
	}
	
	if(strlen(*(argv+2)) > 19) {
		printf("이름은 최대 19자까지 허용됩니다. 중간에 공백 쓰지 않는 것도 유의하세요.\n");
		return 1;
	}
	
	if(strcmp(*(argv+2), "SERVER") == 0) {
		printf("\"SERVER\"는 사용할 수 없는 이름입니다.\n");
		return 1;
	}
	
	strcpy(m_userName, *(argv+2));
  	//fflush(stdout);
	
	printf("클라이언트 프로그램을 시작합니다.\n");
	
  	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);   //서버에 연결할 소켓 생성 및 열기
  	if(sock < 0) {
  	  	fprintf(stderr, "error: socket()\n");
    	return 1;
  	}

  	memset(&echoServAddr, 0, sizeof(echoServAddr));   //구조체의 메모리 클리어
  	echoServAddr.sin_family = AF_INET;   //서버와 통신할 규약 설정(AF_INET/TCP)
  	echoServAddr.sin_addr.s_addr = inet_addr(servIP);   //서버 주소 설정
  	echoServAddr.sin_port = htons(SERVER_PORT);   //사전에 정의된 서버 포트 설정

	//클라이언트가 서버에 접속을 요청한다.
  	iRet = connect(sock, (struct sockaddr*)&echoServAddr, sizeof(echoServAddr));
  	if(iRet < 0) {
 		fprintf(stderr, "error: connect()\n" );
 		close(sock);
 		return 1;
 	}

	printf("서버 접속이 완료되었습니다.\n");

  	while(1)
  	{
    	FD_ZERO(&status);
    	FD_SET(0, &status);   //keyboard를 감시하기 위함(stdin)
    	FD_SET(sock, &status);   //소켓 역시 감시 대상
    	iRet = select(sock + 1, &status, 0, 0, 0);
		if(iRet < 0) {   //select() 실패시 서버에게 "quit"을 보낸다.
	      	fprintf(stderr, "error: select()\n" );
			strcpy(m_message.m_userName, m_userName);
			strcpy(m_message.m_buffer, LOGOUT);
			time(&stTempTime);
	      	strftime(m_message.m_time, 26, "%Y-%m-%d %H:%M:%S", localtime(&stTempTime));
	      	write(sock, &m_message, sizeof(m_message));
	      	break;
	    }
	   	// select의 인자 : 감시하고자 하는 요소, 읽기, 쓰기, 에러, 블로킹(얼마나 정지시킬지/0이면 무한대기 -> 입력이 없으면 밑으로 안내린다.)
    
		//사용자가 글자를 입력하고 있다.
	    if(FD_ISSET(0, &status) == 1)
	    {
	      	iRet = read(0, m_message.m_buffer, sizeof(m_message.m_buffer) - 1);
			if(m_message.m_buffer[iRet - 1] == '\n') m_message.m_buffer[iRet - 1] = '\0';
			m_message.m_buffer[iRet] = '\0';   //문자열을 완성하기 위해 NULL문자를 넣어준다.
	      	time(&stTempTime);
	      	strftime(m_message.m_time, 26, "%Y-%m-%d %H:%M:%S", localtime(&stTempTime));
	      	strcpy(m_message.m_userName, m_userName);

			write(sock, &m_message, sizeof(m_message));
	    }
		//소켓으로부터 메시지를 받음
		else if(FD_ISSET(sock, &status) == 1)
		{
		    read(sock, &m_message, sizeof(m_message));
		    
      		//서버로부터 프로그램 종료를 권장(?)받음
		    if(m_message.m_buffer[0] == CLTEND) break;
			
		    //fflush(stdout);  // 개행 문자가 없으므로 출력되지 않는다.
			printf("from %s[%s] at (%s): %s\n", m_message.m_userName, inet_ntoa(echoServAddr.sin_addr), m_message.m_time, m_message.m_buffer);
		}
 	}

	close(sock);
	printf("클라이언트를 종료합니다.\n");
	return 0;
}