#ifndef _COMMON_H_
#define _COMMON_H_

#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<errno.h>
#include<string.h>
#include<sqlite3.h>
#include<sys/wait.h>
#include<signal.h>
#include<time.h>
#include<pthread.h>
#include<sys/stat.h>
#include<sqlite3.h>

#define STAFF_DATABASE "staff_manage_system.db"
#define USER_LOGIN 0x00000001
#define USER_MODIFY 0x00000002
#define USER_QUERY 0x00000003

#define ADMIN_LOGIN 0x10000000
#define ADMIN_QUERY 0x10000001
#define ADMIN_MODIFY 0x10000002
#define ADMIN_ADDUSER 0x10000004
#define ADMIN_DELUSER 0x10000008
#define ADMIN_HISTORY 0x10000010
#define QUIT 0x11111111

#define ADMIN 0
#define USER 1
typedef struct
{
	int num;//工号
	int usertype;
	char name[16];//姓名
	char passwd[8];//密码
	int age;//年龄
	char telephone[16];
	int wages;//工资
}staff_t;

typedef struct{
	int usertype;//用户类型
	int msgtype;//消息类型
	char recvmsg[128];//通信消息
	char username[16];//用户名
	char passwd[8];//密码	
	staff_t info;
	int flags;
}MSG;
typedef struct thread_data{
	int acceptfd;
	pthread_t thread;
	int states;
	MSG *msg;
	void *prvi_data;
}thread_data_t;

#endif
