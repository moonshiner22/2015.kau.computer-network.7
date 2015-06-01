#include "smart.h"
#include "function.h"

int main(int argc, char *argv[])
{
  fd_set status;
  MESSAGE m_message;
  char m_userName[20];
  
  int sock;
  struct sockaddr_in echoServAddr;  //서버 구조체
  unsigned short echoServPort;  // 서버 포트
  char *servIP;  //서버 주소
  char *echoString;
  char buffer[500];  // 버퍼

  time_t stTempTime;
  int iRet;

  servIP = argv[1];  // 서버 주소
  
  sock = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );  // 서버에 연결할 소켓 생성 및 열기
  if( sock < 0 )  // 실패했을 시...
  {
    printf( "Socket Function Failed!\n" );
    return 0;
  }

  memset( &echoServAddr, 0, sizeof( echoServAddr ) );  // 구조체의 메모리 클리어
  echoServAddr.sin_family = AF_INET;  // 서버와 통신할 규약 설정(AF_INET/TCP)
  echoServAddr.sin_addr.s_addr = inet_addr( servIP );  // 서버 주소 설정
  echoServAddr.sin_port = htons( 9999 );  // 서버 포트 설정

  iRet = connect( sock, ( struct sockaddr* )&echoServAddr, sizeof( echoServAddr ) );
  // 클라이언트가 서버에 접속을 요청한다.
  
  if(iRet < 0)  // 접속 요청 실패
  {
    printf( "Connect Function Failed!\n" );
    close( sock );  // 소켓이 열려 있으므로 닫아준다. 
    return 0;
  }

  printf( "접속하였습니다. 이름을 입력해 주시기 바랍니다. : " );
  fflush( stdout );
  iRet = read( 0, m_userName, sizeof( m_userName ) );
  m_userName[iRet-1] = 0;

  while(1)
  {
    FD_ZERO( &status );
    FD_SET( 0, &status );
    FD_SET( sock, &status );
    iRet = select( sock+1, &status, 0, 0, 0 );

    if( iRet < 0 )  // 셀렉트 문 오류 처리
    {
      printf( "Select Function Error!\n" );
      write( sock, LOGOUT, sizeof(LOGOUT) );
      break;
    }
    // select의 인자 : 감시하고자 하는 요소, 읽기, 쓰기, 에러, 블로킹(얼마나 정지시킬지/0이면 무한대기->입력이 없으면 밑으로 안내린다.) )
    
    if( 1 == FD_ISSET( 0, &status ) )
    {
      iRet = read( 0, m_message.m_buffer, sizeof( m_message.m_buffer ) );  // stdin으로부터 받는 글자를 버퍼에 저장한다.

      time( &stTempTime );
      strftime( m_message.m_time, 26, "%Y-%m-%d %H:%M:%S", localtime( &stTempTime ) );
      
      m_message.m_buffer[iRet-1] = 0;
      
      strcpy( m_message.m_userName, m_userName );

      if( CLTEND == m_message.m_buffer[0] )  // 프로그램 종료 루틴
      {
        printf( "클라이언트를 종료합니다...\n" );
        strcpy( m_message.m_buffer, LOGOUT );
        write( sock, &m_message, iRet );
      }
      
      write( sock, &m_message, sizeof( m_message ) );  // 소켓을 통해 서버에 해당 메시지를 전송한다.  

    }
    else if( 1 == FD_ISSET( sock, &status ) )
    {
      read( sock, &m_message, sizeof( m_message ) );
      
      if( 0 == strcmp( ENDMSG, m_message.m_buffer ) )  // 프로그램 종료 루틴
      {
        printf( "서버가 종료되었으므로 프로그램을 끝냅니다.\n" );
        break;
      }

      printf( "[ %s : ", m_message.m_userName );
      fflush( stdout );  // 개행 문자가 없으므로 출력되지 않는다.
      
      printf( "%s", m_message.m_buffer );

      printf( "(%s)]\n", m_message.m_time );  // 이 때 출력된다.
    }
  }

  close( sock );  //소켓을 닫아준다.
  return 0;

}