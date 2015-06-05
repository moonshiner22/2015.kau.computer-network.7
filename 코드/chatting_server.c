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

void SendStringToClient(int _socket,  const char *_ccp);
const char * itoa(unsigned int _uiData);

int main()
{
	MESSAGE m_message;
  
  	fd_set status;
  
  	int servSock;   //서버소켓번호
 	int clntSock[MAXUSER];  // 클라이언트 소켓
  	int tempSock;

	int iMaxSock;
  
	unsigned int uiUser;

	int iCount;
	int i;
  
	struct sockaddr_in echoServAddr;  //서버 소켓 구조체(신버전)
	struct sockaddr_in echoClntAddr;  //클라이언트 주소...
  
	unsigned short echoServPort = 9999;  // 서버 포트.... 9999...
	unsigned int clntLen;  // 

	time_t stTempTime;
  
  	int iRet;  // 잡동사니를 넣어두는 변수.

  	unsigned char ucBuffer[500];  // 클라이이언트로부터 메시지를 저장하는 버퍼.

 	//echoServPort = 9999;  // 사용할 포트 번호.

 	servSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);  // Socket 생성.
	if(servSock < 0) {
		fprintf(stdout, "error: server socket creation error\n" );
    		return 1;
	}

	memset(&echoServAddr, 0, sizeof(echoServAddr));
  	echoServAddr.sin_family = PF_INET;
  	echoServAddr.sin_addr.s_addr = htonl(INADDR_ANY);  // INADDR_ANY는 자동으로 주소를 입력시켜 준다. 직접 입력해도 되나, 메모리에 저장되는 방식에 유의해야 한다.(Little->Big(표준))
// 서버 주소는 반드시 고정되어야 하지만 클라이언트에서는 클라이언트의 주소를 자동할당하는 것이 좋다. 컴퓨터가 바뀌어도 자동 할당됨.
	echoServAddr.sin_port = htons(echoServPort);  // 포트 번호도 Big-Endian으로 바꿔주어야 한다. short형이므로 htons
  
	iRet = bind(servSock, (struct sockaddr *)&echoServAddr, sizeof(echoServAddr));
  	// 생성된 소켓에 포트 번호, IP등을 부여한다. 바인딩
  	// bind함수에 쓰이는 인자는 구형 구조체이므로 캐스팅한다.
 	// 블로킹 함수가 아니다.
  	if(iRet < 0) {
  	  close(servSock);  // 소켓을 연 이후 에러가 났으므로 닫아주도록 한다.
  	  fprintf(stderr, "error: Bind Error!\n" );
  	  return 1;
  	}

  	iRet = listen(servSock, MAXPENDING); // MAX PENDING : 5
	if(iRet < 0) {  
  	  close(servSock);  // 소켓을 연 이후 에러가 났으므로 닫아주도록 한다.
  	  fprintf(stderr, "error: Listen Error!\n" );
  	  return 1;
  	}

  	clntLen = sizeof(echoClntAddr);
   	iMaxSock = servSock+1;  //셀렉트 문의 첫번째 인자는 무조건 제일 큰 소켓.
  
  	uiUser = 0;
  
  	while(1)
  	{
    		FD_ZERO(&status);  // 변수 초기화(0으로)
    		FD_SET(0, &status);  // 0번째 값을 1로 바꾼다(stdin) -> 키보드 감지.
    		FD_SET(servSock, &status);  // 랑데부 소켓과 키보드만 감지한다.

   	 	for(i = 0; i < uiUser; i++) {
      			FD_SET(clntSock[i], &status);
      			if(iMaxSock <= clntSock[i]) iMaxSock = clntSock[i]+1;
		}
    
    		iRet = select(iMaxSock, &status, 0, 0, 0);
		if(iRet < 0) {
      			fprintf(stderr, "errer: Select Error!\n" );
      			strcpy(m_message.m_buffer, "error: server is destroyed!");
      
     			for(iCount = 0; iCount < uiUser; iCount++) {  // 접속한 클라이언트들에게 전부 전송한다.
       				write(clntSock[iCount], &m_message, sizeof(m_message));
      			}
      
      			strcpy( m_message.m_buffer, "/q" );
      
      			for( iCount=0; iCount<uiUser; iCount++ ) {  // 접속한 클라이언트들에게 전부 전송한다.
        			write(clntSock[iCount], &m_message, sizeof(m_message));
      			}

			break;
    		}
    
    		if(FD_ISSET(servSock, &status) == 1)  // 랑데부. 새로운 클라이언트가 접속했을 때.
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
				fprintf(stderr, "error: Accept Error!\n");
				continue;
		      	}
    
      			printf("클라이언트 접속 IP : %s\n", inet_ntoa( echoClntAddr.sin_addr));
      			// Network to ASCII Code(수치로만 되어있는 IP주소를 사용자가 알아볼 수 있게 문자열로 바꾸어준다.)
  
		      	printf("클라이언트 접속 PORT : %d\n", ntohs( echoClntAddr.sin_port));
		      	// Big-Endian으로 저장된 클라이언트의 포트 정보를 서버에 맞게 Little-Endian으로 변환하여 출력한다.

      			if( MAXUSER <= uiUser ) {  // 사람이 다 찼는데 또 들어올 경우.
        			close( tempSock );
        			continue;  // 와일 문 정상화.
      			}

      			clntSock[uiUser] = tempSock;  // tempSock으로 받은 커뮤니케이션 소켓을 넣어준다.
      			uiUser = uiUser + 1;  // 접속 인원 + 1

     			printf("현재 접속자 수는 총 %d명 입니다.\n", uiUser);
    		}
		else if(FD_ISSET( 0, &status ) == 1)
    		{
		      	iRet = read(0, m_message.m_buffer, sizeof(m_message.m_buffer));
		      	m_message.m_buffer[iRet-1] = 0;
		      	strcpy(m_message.m_userName, "Server");

		      	time(&stTempTime);
		      	strftime(m_message.m_time, 26, "%Y-%m-%d %H:%M:%S", localtime(&stTempTime));
		      
		      	if(m_message.m_buffer[0] == CLTEND) {
				strcpy(m_message.m_buffer, ENDMSG);
				for(iCount = 0; iCount < uiUser; iCount++) {  // 접속한 클라이언트들에게 전부 전송한다.
			  		write(clntSock[iCount], &m_message, sizeof(m_message));
				}

				break;
		      	}
		      
		      	for(iCount = 0; iCount < uiUser; iCount++) {  // 접속한 클라이언트들에게 전부 전송한다.
				write(clntSock[iCount], &m_message, sizeof(m_message));
		      	}
		      
    		}
		else
   		{
      			for(iCount = 0; iCount < uiUser; iCount++)
      			{
        			if(FD_ISSET(clntSock[iCount], &status) == 1)  // 커뮤니케이션.
        			{
				  	iRet = read(clntSock[iCount], &m_message, sizeof(ucBuffer) - 1);
				  	// 커뮤니케이션 소켓으로부터 값을 읽어온다.
				  	// read함수는 글자 수를 반환하므로 iRet에 글자수를 임시로 저장한다.
			      
				  	//ucBuffer[ iRet ] = 0;  // 마지막 글자는 \n(개행문자)이므로 NULL로 처리한다.


				  	//m_message.m_userNumber = iCount+1;

          				if(iRet)
					{
					  	  if(strcmp(LOGOUT, m_message.m_buffer) == 0)
					   	  {
					          	printf("2\n");
              
						      	m_message.m_buffer[0] = CLTEND;
						     	write(clntSock[iCount], &m_message, sizeof(m_message));  // 버퍼에 있는 내용을 클라이언트에게 전송한다..
						      
						      	clntSock[iCount] = clntSock[uiUser-1];
						      	close( clntSock[iCount] );
						      
						      	strcpy( m_message.m_buffer, m_message.m_userName );
						      	printf( "%s\n", m_message.m_buffer );
						      	strcat( m_message.m_buffer, ENDMSG );
						      	printf( "%s\n", m_message.m_buffer );
              
						      	for( i=0; i<uiUser; i++ )  // 접속한 클라이언트들에게 전부 전송한다.
						      	{
								write( clntSock[i], &m_message, sizeof( m_message ) );  // 버퍼에 있는 내용을 클라이언트에게 전송한다..
						      	}

						      	uiUser = uiUser - 1;

						      	printf( "{" OUTMSG "}\n" );
						      	continue;
            					  }
						  else
						  {
						    	printf( "[(%s)%s 님의 말 : ", m_message.m_userName, inet_ntoa( echoClntAddr.sin_addr ) );
						      	fflush( stdout );
						      	printf( "%s", m_message.m_buffer );  // 버퍼에 있는 내용을 stdout(화면)에 출력한다.
						      	printf( "(%s)]\n", m_message.m_time );
						      	//SendStringToClient( clntSock, inet_ntoa( echoClntAddr.sin_addr) );
						      	//SendStringToClient( clntSock, "님의 말 : " );
						      
						      	for( i=0; i<uiUser; i++ )  // 접속한 클라이언트들에게 전부 전송한다.
						      	{
								write( clntSock[i], &m_message, sizeof( m_message ) );  // 버퍼에 있는 내용을 클라이언트에게 전송한다..
						      	}

						   }
          				}
        			}
      			}
  
    		}
  	}// End of while(1)

	printf( "서버를 종료합니다...\n" );
  
	close( servSock );  // 서버 소켓을 종료한다.

	for( i=0; i<uiUser; i++ )  // 접속한 클라이언트들을 전부 닫는다.
	{
    		close( clntSock[i] );  // 클라이언트 소켓을 종료한다.
  	}
  
  	return 0 ;  // 빠빠이
}

void SendStringToClient(int _socket, const char *_ccp) 
{
	write(_socket, _ccp, strlen( _ccp ));
  	return;
}

const char * itoa(unsigned int _uiData)
{
  	char cBuffer[5] = "0000";
  	int i;
  	int j;
  
  	for(i = 10000, j = 0; i >= 0; i = i / 10, j++) {
    		cBuffer[j] = _uiData/i;
    		_uiData = i * (_uiData/i);
  	}

  	return "0000";
}
