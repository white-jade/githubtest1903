#include<stdio.h>
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

#include"common.h"


void do_login(int sockfd);
int admin_and_user_login(int sockfd,MSG *msg);
void admin_menu(int sockfd,MSG *msg);
void admin_query(int sockfd,MSG *msg);
int name_query(int sockfd,MSG *msg);
void all_query(int sockfd,MSG *msg);
void admin_modify(int sockfd,MSG *msg);
int admin_adduser(int sockfd,MSG *msg);
void admin_deluser(int sockfd,MSG *msg);
int admin_queryhistory(int sockfd,MSG *msg);
void sys_close(int sockfd,MSG *msg);
void user_query(int sockfd,MSG *msg);
void user_modify(int sockfd,MSG *msg);
void user_menu(int sockfd,MSG *msg);


void do_login(int sockfd)
{
	int no;
	MSG msg;
	while(1)
	{
		printf("****************************************\n");
		printf("***1:管理员模式 2:普通用户模式 3:退出***\n");
		printf("****************************************\n");
		printf("请输入您选择的数字>>");
		scanf("%d",&no);
		getchar();
		switch(no)
		{
		case 1:
			msg.msgtype = ADMIN_LOGIN;
			msg.usertype = ADMIN;
			break;
		case 2:
			msg.msgtype = USER_LOGIN;
			msg.usertype = USER;
			break;
		case 3:				
			msg.msgtype = QUIT;
			send(sockfd,&msg,sizeof(MSG),0);
			close(sockfd);
			exit(0);
		default:
			printf("您输入有误，请重新输入！\n");
			break;
		}
		admin_and_user_login(sockfd,&msg);
	}
}
int admin_and_user_login(int sockfd,MSG *msg)
{
	memset(msg->username,0,16);
	printf("请输入用户名:");
	scanf("%s",msg->username);
	getchar();
	memset(msg->passwd,0,128);
	printf("请输入密码(6位):");
	scanf("%s",msg->passwd);
	getchar();

	send(sockfd,msg,sizeof(MSG),0);
	recv(sockfd,msg,sizeof(MSG),0);
	printf("msg->recvmsg:%s\n",msg->recvmsg);
	if(strncmp(msg->recvmsg,"OK",2)==0)
	{
		if(msg->usertype == ADMIN)
		{
			printf("亲爱的管理员，欢迎您登录员工管理系统！\n");
			admin_menu(sockfd,msg);
		}else if(msg->usertype == USER)
		{
			printf("亲爱的用户，欢迎您登录员工管理系统！\n");
			user_menu(sockfd,msg);
		}else
		{
			printf("登录失败！%s\n",msg->recvmsg);
			return 0;
		}
	}
}
void admin_menu(int sockfd,MSG *msg)
{
	int no;
	while(1)
	{
		printf("**********************************************************\n");
		printf("*1:查询 2:修改 3:添加用户 4:删除用户 5:查询历史记录 6:退出*\n");
		printf("**********************************************************\n");
		printf("请输入您选择的数字>>");
		scanf("%d",&no);
		getchar();
		switch(no)
		{
			case 1:
				admin_query(sockfd,msg);
				break;
			case 2:
				admin_modify(sockfd,msg);
				break;
			case 3:
				admin_adduser(sockfd,msg);
				break;
			case 4:
				admin_deluser(sockfd,msg);
				break;
			case 5:
				admin_queryhistory(sockfd,msg);
				break;
			case 6:
				sys_close(sockfd,msg);
			default:
				printf("您输入有误，请重新输入！\n");
				break;

		}
	}
}
void admin_query(int sockfd,MSG *msg)
{
	msg->msgtype = ADMIN_QUERY;
	int no;
	while(1)
	{
		printf("**************************************\n");
		printf("***1:按姓名查询 2：查询所有 3：退出***\n");
		printf("**************************************\n");
		printf("请输入您选择的数字>>");
		scanf("%d",&no);
		getchar();
		switch(no)
		{
			case 1:
				name_query(sockfd,msg);//按姓名查询
				break;
			case 2:
				all_query(sockfd,msg);//查询所有
				break;
			case 3:
				admin_menu(sockfd,msg);
			default:
				printf("您输入有误，请重新输入！\n");
				break;
		}
	}

}
int name_query(int sockfd,MSG *msg)
{
	printf("请输入您要查找的用户名:");
	scanf("%s",msg->info.name);
	getchar();
	send(sockfd,msg,sizeof(MSG),0);
	recv(sockfd,msg,sizeof(MSG),0);
	printf("工号\t用户类型\t 姓名\t密码\t年龄\t电话\t工资\n");
	printf("%s.\n",msg->recvmsg);
	printf("查找结束！\n");
	return 0;
}
void all_query(int sockfd,MSG *msg)
{
	send(sockfd,msg,sizeof(MSG),0);
	while(1)
	{
		recv(sockfd,msg,sizeof(MSG),0);
		printf("工号\t用户类型\t 姓名\t密码\t年龄\t电话\t工资\n");
		if(strncmp(msg->recvmsg,"over",4)==0)
		{
			break;
		}
		printf("%s.\n",msg->recvmsg);
	}	
	printf("查找结束！\n");
}
void admin_modify(int sockfd,MSG *msg)
{
	msg->msgtype = ADMIN_MODIFY;
	int n;
	memset(&msg->info,0,sizeof(staff_t));
	printf("请输入您要修改员工的工号:");
	scanf("%d",&(msg->info.num));
	getchar();
	printf("*************请输入要修改的选项******************\n");
	printf("****1：姓名 2：年龄 3：电话 4:工资 5：退出******\n");
	printf("***********************************************\n");
	printf("请输入要修改的选项输入您的选择（数字）>>");
	scanf("%d",&n);
	getchar();
	switch(n)
	{
	case 1:
		printf("请输入用户名：");
		msg->recvmsg[0] = 'N';
		scanf("%s",msg->info.name);
		break;
	case 2:
		printf("请输入年龄:");
		msg->recvmsg[0] = 'A';
		scanf("%d",&(msg->info.age));
		break;
	case 3:
		printf("请输入电话:");
		msg->recvmsg[0] = 'P'; 
		scanf("%s",msg->info.telephone);
		break;
	case 4:
		printf("请输入工资:");
		msg->recvmsg[0] = 'S';
		scanf("%d",&(msg->info.wages));
		break;
	case 5:
		return;
	default:
		printf("您输入错误，请重新输入！\n");
		break;
	}
}

int admin_adduser(int sockfd,MSG *msg)
{
	msg->usertype = ADMIN;
	msg->msgtype = ADMIN_ADDUSER;
	char answer;
	char temp;
	memset(&msg->info,0,sizeof(staff_t));
	while(1)
	{
		printf("***********热烈欢迎新员工***********\n");
		printf("请输入工号:");
		scanf("%d",&msg->info.num);
		getchar();
		printf("您输入的工号是:%d\n",msg->info.num);
		printf("工号一旦录入无法更改，请确认是否正确(y/n)");
		scanf("%c",&answer);
		getchar();
		if(answer == 'y')
		{
			printf("请输入用户名:");
			scanf("%s",msg->info.name);
			getchar();
			printf("请输入用户密码:");
			scanf("%s",msg->info.passwd);
			getchar();
			printf("请输入年龄:");
			scanf("%d",&(msg->info.age));
			getchar();
			printf("请输入电话:");
			scanf("%s",msg->info.telephone);
			getchar();
			printf("请输入工资:");
			scanf("%d",&(msg->info.wages));
			getchar();
			printf("是否为管理员：(Y/N)");
			scanf("%c",&temp);
			getchar();
			if(temp == 'Y' || temp == 'y')
				msg->info.usertype = ADMIN;
			else if(temp == 'N' || temp == 'n')
				msg->info.usertype = USER;
			printf("msg->info.usertype:%d\n",msg->info.usertype);
		}else{
			printf("请重新添加用户名\n");
		}
		send(sockfd,msg,sizeof(MSG),0);
		recv(sockfd,msg,sizeof(MSG),0);
		if(strncmp(msg->recvmsg,"OK",2)==0)
		{
			printf("添加成功！\n");
			return 0;
		}else{
			printf("%s\n",msg->recvmsg);
		}
		printf("是否继续添加员工:(Y/N)");
		scanf("%c",&temp);
		getchar();
		if(temp == 'N' || temp == 'n')
			break;
	}
}

void admin_deluser(int sockfd,MSG *msg)
{
	msg->msgtype = ADMIN_DELUSER;
	printf("请输入要删除的用户工号:");
	scanf("%d",&(msg->info.num));
	getchar();
	printf("请输入要删除的用户名:");
	scanf("%s",msg->info.name);
	getchar();
	send(sockfd,msg,sizeof(MSG),0);
	recv(sockfd,msg,sizeof(MSG),0);
	if(strncmp(msg->recvmsg,"OK",2)==0)
	{
		printf("数据库修改成功！删除工号为%d\n",msg->info.num);
	}
}
int admin_queryhistory(int sockfd,MSG *msg)
{
	msg->msgtype = ADMIN_HISTORY;
	send(sockfd,msg,sizeof(MSG),0);
	while(1)
	{
		recv(sockfd,msg,sizeof(MSG),0);
		if(strncmp(msg->recvmsg,"over",4)==0)
		{
			break;
		}
		printf("%s.\n",msg->info);
	}
	printf("查询历史记录结束！\n");
}
void sys_close(int sockfd,MSG *msg)
{
	msg->msgtype = QUIT;
	send(sockfd,msg,sizeof(MSG),0);
	close(sockfd);
	exit(0);
}


void user_menu(int sockfd,MSG *msg)
{
	int no;
	printf("************************************\n");
	printf("********1:查询 2:修改 3:退出********\n");
	printf("************************************\n");
	printf("请输入您选择的数字>>");
	scanf("%d",&no);
	getchar();
	switch(no)
	{
		case 1:
			user_query(sockfd,msg);
			break;
		case 2:
			user_modify(sockfd,msg);
			break;
		case 3:
			sys_close(sockfd,msg);
		default:
			printf("您输入有误，请重新输入！\n");
			break;
	}
}
void user_query(int sockfd,MSG *msg)
{
	msg->msgtype = USER_QUERY;
	send(sockfd,msg,sizeof(MSG),0);
	recv(sockfd,msg,sizeof(MSG),0);
	printf("工号\t用户类型\t 姓名\t密码\t年龄\t电话\t工资\n");
	printf("%s",msg->recvmsg);
}
void user_modify(int sockfd,MSG *msg)
{
	msg->msgtype = USER_MODIFY;
	int numb;
	printf("请输入您要修改的工号:");
	scanf("%d",&msg->info.num);
	getchar();
	printf("**请输入要修改的选项(其他询问管理员)**\n");
	printf("******1:电话 2:密码 3:退出*******\n");
	printf("*************************************\n");
	printf("请输入您选择的数字>>");
	scanf("%d",&numb);
	getchar();
	switch(numb)
	{
	case 1:
		printf("请输入电话:");
		msg->recvmsg[0] = 'P';
		scanf("%s",msg->info.telephone);
		getchar();
		break;
	case 2:
		printf("请输入密码:");
		msg->recvmsg[0] = 'M';
		scanf("%s",msg->info.passwd);
		getchar();
		break;
	case 3:
		return;
	default:
		printf("您输入有误，请重新输入！\n");
		break;				
	}
	send(sockfd, msg, sizeof(MSG), 0);
	recv(sockfd, msg, sizeof(MSG), 0);
	printf("%s",msg->recvmsg);
	printf("修改结束.修改结束\n");
}


int main(int argc, const char *argv[])
{
	int sockfd;
	int acceptfd;
	ssize_t recvbytes,sendbytes;
	struct sockaddr_in clientaddr;
	struct sockaddr_in serveraddr;
	socklen_t serlen = sizeof(serveraddr);
	socklen_t clilen = sizeof(clientaddr);
	sockfd = socket(AF_INET,SOCK_STREAM,0);
	if(sockfd < 0)
	{
		printf("fail to socket");
	}
	printf("sockfd:%d\n",sockfd);
	memset(&serveraddr,0,sizeof(serveraddr));
	memset(&clientaddr,0,sizeof(clientaddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(5001);
	serveraddr.sin_addr.s_addr = inet_addr("192.168.10.107");
	if(connect(sockfd,(const struct sockaddr *)&serveraddr,serlen)<0)
	{
		perror("fail to connect\n");
		exit(-1);
	}
	do_login(sockfd);
	close(sockfd);

	return 0;
}
