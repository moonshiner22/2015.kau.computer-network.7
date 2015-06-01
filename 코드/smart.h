#ifndef __SMART_H__
#define __SMART_H__

#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include "function.h"

#define RCVBUFSIZE 32
#define ENDMSG "@ENDM"
#define LOGOUT "quit"
#define CLTEND 0x05
#define OUTMSG " 님께서 퇴장하셨습니다."

#define MAXPENDING 5 // 최대 허용 대기 가능 인원
#define MAXUSER 10 // 최대 허용 접속 가능 인원.

#endif
