#include <stdio.h>
#include <sys/types.h>        
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/select.h>
#include <pthread.h>
#include <sqlite3.h>

#include "common.h"

sqlite3 *db;  //仅服务器使用
int flags = 0;
void get_time(char *timedata)
{
	time_t mytime;
	struct tm *mytm;
	mytime = time(NULL);
	mytm = localtime(&mytime);
	sprintf(timedata,"%d-%d-%d %d:%d:%d",mytm->tm_year+1900,mytm->tm_mon+1,mytm->tm_mday,\
			mytm->tm_hour,mytm->tm_sec,mytm->tm_min,mytm->tm_sec);
}
void history_init(MSG *msg,char *buf)
{
	int nrow,ncolumn;
	char *errmsg = NULL;
	char **dbResult;
	char sqlhistory[128] = {0};
	char timedata[128] = {0};
	get_time(timedata);
	printf(sqlhistory,"insert into historyinfo values ('%s','%s','%s');",timedata,msg->username,buf);
	if(sqlite3_exec(db,sqlhistory,NULL,NULL,&errmsg)!= SQLITE_OK){
		printf("%s.\n",errmsg);
		printf("insert historyinfo failed.\n");
	}else{
		printf("includesert historyinfo success.\n");
	}
}
int process_user_or_admin_login_request(int acceptfd,MSG *msg)
{
	char sql[128] = {0};
	char *errmsg = NULL;//用来存储错误信息字符串
	char **dbResult;
	int nrow,ncolumn;//nrow查找出的总行数  ncolumn存储列
	
	msg->info.usertype =  msg->usertype;
	strcpy(msg->info.name,msg->username);
	strcpy(msg->info.passwd,msg->passwd);
	
	printf("usertype: %#x-----usrname: %s---passwd: %s.\n",msg->info.usertype,msg->info.name,msg->info.passwd);
	sprintf(sql,"select * from userinfo where usertype=%d and name='%s' and passwd='%s';",msg->info.usertype,msg->info.name,msg->info.passwd);
	if(sqlite3_get_table(db,sql,&dbResult,&nrow,&ncolumn,&errmsg) != SQLITE_OK){
	//比较从sqlite里面读出来的字符串数据是否相同
		printf("%s.\n",errmsg);		
	}else{	
		if(nrow == 0){
			strcpy(msg->recvmsg,"name or passwd failed.\n");
			send(acceptfd,msg,sizeof(MSG),0);
		}else{
			strcpy(msg->recvmsg,"OK");
			send(acceptfd,msg,sizeof(MSG),0);
		}
	}
	return 0;	
}

int process_user_modify_request(int acceptfd,MSG *msg)
{ 
	char *errmsg = NULL;//用来存储错误信息字符串
	char **dbResult;
	int nrow,ncolumn;//nrow查找出的总行数  ncolumn存储列
	char sql[128] = {0};
	char historybuf[128] = {0};
	switch(msg->recvmsg[0])
	{
	case 'P':
		sprintf(sql,"update userinfo set telephone = '%s' where num = '%d';",msg->info.telephone,msg->info.num);
		sprintf(historybuf,"'%s'修改工号为'%d'的电话为'%s'",msg->username,msg->info.num,msg->info.telephone);
		break;
	case 'M':
		sprintf(sql,"update userinfo set passwd = '%s' where num = '%d';",msg->info.passwd,msg->info.num);
		sprintf(historybuf,"'%s'修改工号为'%d'的电话为'%s'",msg->username,msg->info.num,msg->info.passwd);
		break;
	}
	printf("msgtype :%#x--usrtype: %#x--usrname: %s-passwd: %s.\n",msg->msgtype,msg->info.usertype,msg->info.name,msg->info.passwd);
	printf("msg->info.num :%d\t msg->info.telephone: %s.\n",msg->info.num,msg->info.telephone);
	//操作数据库
	if(sqlite3_exec(db,sql,NULL,NULL,&errmsg)!=SQLITE_OK)
	{
		printf("%s\n",errmsg);
		sprintf(msg->recvmsg,"数据库修改失败！%s\n",errmsg);
	}else{
		sprintf(msg->recvmsg,"数据库修改成功");
		history_init(msg,historybuf);
	}
	//通知用户修改信息成功
	send(acceptfd,msg,sizeof(MSG),0);
	printf("%s\n",historybuf);
	return 0;
}

int process_user_query_request(int acceptfd,MSG *msg)
{
	int i = 0,j = 0;
	char sql[128] = {0};
	char **resultp;
	int nrow,ncolumn;
	char *errmsg;
	sprintf(sql,"select * from userinfo where name = '%s';",msg->username);

	if(sqlite3_get_table(db, sql, &resultp,&nrow,&ncolumn,&errmsg) != SQLITE_OK){
		printf("%s.\n",errmsg);
	}else{
		printf("searching.....\n");
		for(i = 0; i < ncolumn; i ++){
			printf("%-8s ",resultp[i]);
		}
		puts("");
		puts("======================");
		int index = ncolumn;
		for(i = 0; i < nrow; i ++){
			printf("%s    %s     %s     %s     %s     %s     %s \n",resultp[index+ncolumn-7],resultp[index+ncolumn-6],resultp[index+ncolumn-5],resultp[index+ncolumn-4],resultp[index+ncolumn-3],resultp[index+ncolumn-2],resultp[index+ncolumn-1]);
			sprintf(msg->recvmsg,"%s,    %s,    %s,    %s,    %s,    %s,    %s;",resultp[index+ncolumn-7],resultp[index+ncolumn-6],resultp[index+ncolumn-5],resultp[index+ncolumn-4],resultp[index+ncolumn-3],resultp[index+ncolumn-2],resultp[index+ncolumn-1]);
			send(acceptfd,msg,sizeof(MSG),0);
			//usleep(1000);
			puts("================");
			index += ncolumn;
		}
		sqlite3_free_table(resultp);
		printf("sqlite3_get_table successssfully.\n");
	}
}


int process_admin_modify_request(int acceptfd,MSG *msg)
{
	int nrow,ncolumn;
	char *errmsg, **resultp;
	char sql[128] = {0};
	char historybuf[128] = {0};
	switch(msg->recvmsg[0])
	{
	case 'N':
		sprintf(sql,"update userinfo set name='%s' where num=%d;",msg->info.name, msg->info.num);
		sprintf(historybuf,"%s修改工号为%d的用户名为%s",msg->username,msg->info.num,msg->info.name);
		break;
	case 'A':
		sprintf(sql,"update userinfo set age='%d' where num=%d;",msg->info.age, msg->info.num);
		sprintf(historybuf,"%s修改工号为%d的年龄为%d",msg->username,msg->info.num,msg->info.age);
		break;
	case 'P':
		sprintf(sql,"update userinfo set telephone='%s' where num=%d;",msg->info.telephone, msg->info.num);
		sprintf(historybuf,"%s修改工号为%d的电话为%s",msg->username,msg->info.num,msg->info.telephone);
		break;
	case 'S':
		sprintf(sql,"update userinfo set wages='%d' where num=%d;",msg->info.wages, msg->info.num);
		sprintf(historybuf,"%s修改工号为%d的工资为%d",msg->username,msg->info.num,msg->info.wages);
		break;
	}
	if(sqlite3_exec(db,sql,NULL,NULL,&errmsg) != SQLITE_OK){
		printf("%s.\n",errmsg);
		sprintf(msg->recvmsg,"数据交互据库修改失败！%s", errmsg);
	}else{
		printf("the database is updated successfully.\n");
		sprintf(msg->recvmsg, "数据库修改成功!");
		history_init(msg,historybuf);
	}
	send(acceptfd,msg,sizeof(MSG),0);
	printf("------%s.\n",historybuf);
	return 0;
}


int process_admin_adduser_request(int acceptfd,MSG *msg)
{
	char sql[128] = {0};
	char buf[128] = {0};
	char *errmsg;
	printf("%d\t %d\t %s\t %s\t %d\n %s\t %d\t.\n",msg->info.num,msg->info.usertype,msg->info.name,msg->info.passwd,\
			msg->info.age,msg->info.telephone,msg->info.wages);
	sprintf(sql,"insert into userinfo values('%d','%d','%s','%s','%d','%s','%d');",msg->info.num,msg->info.usertype,msg->info.name,msg->info.passwd,\
			msg->info.age,msg->info.telephone,msg->info.wages);
	if(sqlite3_exec(db,sql,NULL,NULL,&errmsg)!= SQLITE_OK){
		printf("------printf----%s.\n",errmsg);
		strcpy(msg->recvmsg,"failed");
		send(acceptfd,msg,sizeof(MSG),0);
		return -1;
	}else{
		strcpy(msg->recvmsg,"OK");
		send(acceptfd,msg,sizeof(MSG),0);
		printf("%s register success.\n",msg->info.name);
	}

	sprintf(buf,"管理员%s添加了%s用户",msg->username,msg->info.name);
	history_init(msg,buf);

	return 0;
}
int process_admin_deluser_request(int acceptfd,MSG *msg)
{
	char sql[128] = {0};
	char buf[128] = {0};
	char *errmsg;

	printf("msg->info.num :%d\t msg->info.name: %s.\n",msg->info.num,msg->info.name);

	sprintf(sql,"delete from userinfo where num=%d and name='%s';",msg->info.num,msg->info.name);
	if(sqlite3_exec(db,sql,NULL,NULL,&errmsg)!= SQLITE_OK){
		printf("----------%s.\n",errmsg);
		strcpy(msg->recvmsg,"failed");
		send(acceptfd,msg,sizeof(MSG),0);
		return -1;
	}else{
		strcpy(msg->recvmsg,"OK");
		send(acceptfd,msg,sizeof(MSG),0);
		printf("%s deluser %s success.\n",msg->info.name,msg->info.name);
	}

	sprintf(buf,"管理员%s删除了%s用户",msg->username,msg->info.name);
	history_init(msg,buf);

	return 0;
}


int process_admin_query_request(int acceptfd,MSG *msg)
{
	int i = 0,j = 0;
	char sql[128] = {0};
	char **resultp;
	int nrow,ncolumn;
	char *errmsg;

	if(msg->flags == 1){
		sprintf(sql,"select * from userinfo where name='%s';",msg->info.name);
	}else{
		sprintf(sql,"select * from userinfo;");
	}

	if(sqlite3_get_table(db, sql,&resultp,&nrow,&ncolumn,&errmsg) != SQLITE_OK){
		printf("%s.\n",errmsg);
	}else{
		printf("searching.....\n");
		printf("ncolumn :%d\tnrow :%dearchingd.\n",ncolumn,nrow);

		for(i = 0; i < ncolumn; i ++){
			printf("%-8s ",resultp[i]);
		}
		getchar();

		int index = ncolumn;
		for(i = 0; i < nrow; i ++){
			printf("%s    %s     %s     %s     %s     %s     %s \n",resultp[index+ncolumn-7],resultp[index+ncolumn-6],resultp[index+ncolumn-5],\
					resultp[index+ncolumn-4],resultp[index+ncolumn-3],resultp[index+ncolumn-2],resultp[index+ncolumn-1]);
			sprintf(msg->recvmsg,"%s,    %s,    %s,    %s,    %s,    %s,    %s;",resultp[index+ncolumn-7],resultp[index+ncolumn-6],resultp[index+ncolumn-5],\
					resultp[index+ncolumn-4],resultp[index+ncolumn-3],resultp[index+ncolumn-2],resultp[index+ncolumn-1]);
			send(acceptfd,msg,sizeof(MSG),0);
			usleep(1000);
			index += ncolumn;
		}
		if(msg->flags != 1){  //全部查询的时候不知道何时结束，需要手动发送结束标志位，但是按人名查找不需要
			//通知对方查询结束了
			strcpy(msg->recvmsg,"over*");
			send(acceptfd,msg,sizeof(MSG),0);
			}
			sqlite3_free_table(resultp);
			printf("sqlite3_get_table successfully.\n");
	}
}

int history_callback(void *arg, int ncolumn, char **f_value, char **f_name)
{
	int i = 0;
	MSG *msg= (MSG *)arg;
	int acceptfd = msg->flags;

	if(flags == 0){
		for(i = 0; i < ncolumn; i++){
			printf("%-11s", f_name[i]);
		}
		putchar(10);
		flags = 1;
	}

	for(i = 0; i < ncolumn; i+=3)
	{
		printf("%s-%s-%s",f_value[i],f_value[i+1],f_value[i+2]);
		sprintf(msg->recvmsg,"%s---%s---%s.\n",f_value[i],f_value[i+1],f_value[i+2]);
		send(acceptfd,msg,sizeof(MSG),0);
		usleep(1000);
	}
	puts("");

	return 0;
}

int process_admin_history_request(int acceptfd,MSG *msg)
{
	char sql[128] = {0};
	char *errmsg;
	msg->flags = acceptfd;
	sprintf(sql,"select * from historyinfo;");
	if(sqlite3_exec(db,sql,history_callback,(void *)msg,&errmsg) != SQLITE_OK){
		printf("%s.\n",errmsg); 
	}else{
		printf("query history record done.\n");
	}
	strcpy(msg->recvmsg,"over*");
	send(acceptfd,msg,sizeof(MSG),0);
	flags = 0;
}


int process_client_quit_request(int acceptfd,MSG *msg)
{
	printf("------------%s-----------%d.\n",__func__,__LINE__);
}


int process_client_request(int acceptfd,MSG *msg)
{
	switch (msg->msgtype)
	{
		case USER_LOGIN:
		case ADMIN_LOGIN:
			process_user_or_admin_login_request(acceptfd,msg);
			break;
		case USER_MODIFY:
			process_user_modify_request(acceptfd,msg);
			break;
		case USER_QUERY:
			process_user_query_request(acceptfd,msg);
			break;
		case ADMIN_MODIFY:
			process_admin_modify_request(acceptfd,msg);
			break;

		case ADMIN_ADDUSER:
			process_admin_adduser_request(acceptfd,msg);
			break;

		case ADMIN_DELUSER:
			process_admin_deluser_request(acceptfd,msg);
			break;
		case ADMIN_QUERY:
			process_admin_query_request(acceptfd,msg);
			break;
		case ADMIN_HISTORY:
			process_admin_history_request(acceptfd,msg);
			break;
		case QUIT:
			process_client_quit_request(acceptfd,msg);
			break;
		default:
			break;
	}

}


int main(int argc, const char *argv[])
{
	int sockfd;
	int acceptfd;
	ssize_t recvbytes;
	struct sockaddr_in serveraddr;
	struct sockaddr_in clientaddr;
	socklen_t serlen = sizeof(serveraddr);
	socklen_t clilen = sizeof(clientaddr);

	MSG msg;
	char *errmsg;

	if(sqlite3_open(STAFF_DATABASE,&db) != SQLITE_OK){
		printf("%s.\n",sqlite3_errmsg(db));
	}else{
		printf("the database open success.\n");
	}

	if(sqlite3_exec(db,"create table userinfo(num int,usertype int,name char,passwd char,age int,telephone char,wages int);",NULL,NULL,&errmsg)!= SQLITE_OK){
		printf("%s.\n",errmsg);
	}else{
		printf("create userinfo table success.\n");
	}

	if(sqlite3_exec(db,"create table historyinfo(time text,name text,words text);",NULL,NULL,&errmsg)!= SQLITE_OK){
		printf("%s.\n",errmsg);
	}else{ 
		printf("create historyinfo table success.\n");
	}

	//创建网络通信的套接字
	sockfd = socket(AF_INET,SOCK_STREAM, 0);
	if(sockfd == -1){
		perror("socket failed.\n");
		exit(-1);
	}
	printf("sockfd :%d.\n",sockfd); 
	
	//int b_reuse = 1;
	//setsockopt (sockfd, SOL_SOCKET, SO_REUSEADDR, &b_reuse, sizeof (int));
	
	//填充网络结构体
	memset(&serveraddr,0,sizeof(serveraddr));
	memset(&clientaddr,0,sizeof(clientaddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port   = htons(5001);
	serveraddr.sin_addr.s_addr = inet_addr("192.168.10.107");


	//绑定网络套接字和网络结构体
	if(bind(sockfd, (const struct sockaddr *)&serveraddr,serlen) == -1)
	{
		printf("bind failed.\n");
		exit(-1);
	}

	//监听套接字，将主动套接字转化为被动套接字
	if(listen(sockfd,10) == -1)
	{
		printf("listen failed.\n");
		exit(-1);
	}

	//定义一张表
	fd_set readfds,tempfds;
	//清空表
	FD_ZERO(&readfds);
	FD_ZERO(&tempfds);
	//添加要监听的事件
	FD_SET(sockfd,&readfds);
	int nfds = sockfd;
	int retval;
	int i = 0;
	while(1){
		tempfds = readfds;
		//记得重新添加
		retval =select(nfds + 1, &tempfds, NULL,NULL,NULL);
		//判断是否是集合里关注的事件
		for(i = 0;i < nfds + 1; i ++){
			if(FD_ISSET(i,&tempfds)){
				if(i == sockfd){
					//数据交互 
					acceptfd = accept(sockfd,(struct sockaddr *)&clientaddr,&clilen);
					if(acceptfd == -1){
						printf("acceptfd failed.\n");
						exit(-1);
					}
					printf("ip : %s.\n",inet_ntoa(clientaddr.sin_addr));
					FD_SET(acceptfd,&readfds);
					nfds = nfds > acceptfd ? nfds : acceptfd;
				}else{
					recvbytes = recv(i,&msg,sizeof(msg),0);
					printf("msg.type :%#x.\n",msg.msgtype);
					if(recvbytes == -1){
						printf("recv failed.\n");
						continue;
					}else if(recvbytes == 0){
						printf("peer shutdown.\n");
						close(i);
						FD_CLR(i, &readfds);  //删除集合中的i
					}else{
						process_client_request(i,&msg);
					}
				}
			}
		}
	}
	close(sockfd);

	return 0;
}














