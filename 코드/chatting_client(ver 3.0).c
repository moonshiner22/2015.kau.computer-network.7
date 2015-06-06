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
  	char * servIP;  //서버 주소
  	char * echoString;
  	char buffer[500];  // 버퍼

  	time_t stTempTime;
  	int iRet;

  	servIP = *(argv+1);  // 서버IP

  	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);   //서버에 연결할 소켓 생성 및 열기
  	if(sock < 0) {
  	  	fprintf(stderr, "error: socket()\n");
    		return 1;
  	}

  	memset(&echoServAddr, 0, sizeof(echoServAddr));  // 구조체의 메모리 클리어
  	echoServAddr.sin_family = AF_INET;  // 서버와 통신할 규약 설정(AF_INET/TCP)
  	echoServAddr.sin_addr.s_addr = inet_addr(servIP);  // 서버 주소 설정
  	echoServAddr.sin_port = htons(9999);  // 사전에 정의된 서버 포트 설정

	// 클라이언트가 서버에 접속을 요청한다.
  	iRet = connect(sock, (struct sockaddr*)&echoServAddr, sizeof(echoServAddr));
  	if(iRet < 0) {
 		  fprintf(stderr, "error: connect()\n" );
 		  close(sock);
 		  return 1;
 	}

  	printf("접속하였습니다. 먼저 이름을 입력해 주시기 바랍니다.: " );
  	fflush(stdout);
  	iRet = read(0, m_userName, sizeof(m_userName));
  	m_userName[iRet - 1] = 0;   //문자열을 완성하기 위해 NULL문자를 넣어준다.

  	while(1)
  	{
    		FD_ZERO(&status);
    		FD_SET(0, &status);   //keyboard를 감시하기 위함(stdin)
    		FD_SET(sock, &status);   //소켓 역시 감시 대상
    		iRet = select(sock + 1, &status, 0, 0, 0);
		if(iRet < 0) {   //select() 실패시 서버에게 "quit"을 보낸다.
	      		fprintf(stderr, "error: select()\n" );
	      		write(sock, LOGOUT, sizeof(LOGOUT));
	      		break;
	    	}
	   	// select의 인자 : 감시하고자 하는 요소, 읽기, 쓰기, 에러, 블로킹(얼마나 정지시킬지/0이면 무한대기 -> 입력이 없으면 밑으로 안내린다.)
    
		//사용자가 글자를 입력하고 있다.
	    	if(FD_ISSET(0, &status) == 1)
	    	{
	      		iRet = read(0, m_message.m_buffer, sizeof(m_message.m_buffer));
	      		time( &stTempTime );
	      		strftime(m_message.m_time, 26, "%Y-%m-%d %H:%M:%S", localtime(&stTempTime));
	      		m_message.m_buffer[iRet - 1] = 0;   //문자열을 완성하기 위해 NULL문자를 넣어준다.
	      		strcpy(m_message.m_userName, m_userName);

			write(sock, &m_message, sizeof(m_message));
	    	}
		else if(FD_ISSET(sock, &status) == 1)   //소켓으로부터 메시지를 받음
		{
		      read(sock, &m_message, sizeof(m_message));
		      
		      // 프로그램 종료
		      if(strcmp(ENDMSG, m_message.m_buffer) == 0) {
				printf("server is destroyed\n");
				break;
		      }

      		      //프로그램 종료
		      if(m_message.m_buffer[0] == CLTEND) {
		      		printf("정상적으로 로그아웃 되었습니다.\n");
				strcpy(m_message.m_buffer, LOGOUT);
				write(sock, &m_message, iRet);
				break;
		      }

		      //m_message.m_userName[strlen(m_message.m_userName)] = '\0';
		      //m_message.m_buffer[strlen(m_message.m_buffer)] = '\0';
		      //m_message.m_time[strlen(m_message.m_time)] = '\0';
		      printf("from %s: ", m_message.m_userName);
		      fflush(stdout);  // 개행 문자가 없으므로 출력되지 않는다.
		      printf("%s ", m_message.m_buffer);
		      printf("(%s)\n", m_message.m_time);  //시간 출력.
		}
 	}

	close(sock);
	return 0;
}
/*
void MyMemorySet( void *vp, unsigned char ucPad, unsigned int uiSize )
{
  while( uiSize != 0 )
  {
    *( ( unsigned char * )vp + uiSize ) = ucPad;
    --uiSize;
    vp = ( unsigned char * )vp + 1;  // Void Pointer는 값은 자유로 가르킬 수 있지만, 연산이 불가능하므로 Casting.
  }
  return;
}

void MyBZero( void *vp, unsigned int uiSize )
{
  MyMemorySet( vp, 0x00, uiSize );
  return;
}*/
