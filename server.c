#include "server_util.h"
#include "server_pi.h"
#include "server_cm.h"

#include <sys/socket.h>
#include <netinet/in.h>

#include <unistd.h>
#include <errno.h>

#include <string.h>
#include <memory.h>
#include <stdlib.h>
#include <stdio.h>

int listenfd;
int listenPort = 6789;

struct Client* head, *tail;
int connCnt = 0;

char reqBuf[MAXBUF];

struct fd_set fds;
struct timeval tv;

const char promptClientListen[] = "Server starts listening at port: %d.\r\n";
const char promptDataListen[] = "Server starts data listening at port: %d for client (fd: %d, pasvlfd: %d).\r\n";
const char promptTimeout[] = "Select Timeout.\r\n";
const char promptClientClose[] = "Connection with client (fd: %d) closed normally.\r\n";
const char promptReceive[] = "Receive from client (fd: %d):\r\n";
const char promptNewConn[] = "New connection from client (fd: %d).\r\n";
const char promptNewDataConn[] = "New data connection from client (fd: %d, pasvlfd: %d, pasvrfd: %d).\r\n";

const char errorListenFail[] = "Fail to setup listen at port: %d!\r\n";
const char errorSelect[] = "Error occurs when using select()! Sever aborts!\r\n";
const char errorClientConn[] = "Connection with client fails!\r\n";
const char errorClientDataConn[] = "Data Connection with client (fd: %d, pasvlfd: %d) fails!\r\n";
const char errorInvalidReq[] = "Invalid Request!\r\n";
const char errorConnShutDown[] = "Connection with client (fd: %d) has been shut down!\r\n";

struct Client* getClientHead() {
	return head;
}

struct Client* deleteClientByfd(int fd) {
	struct Client* client = getClientByfd(fd);
	
	if (!client) return NULL;
	if (client->pasvlfd != -1) {
		close(client->pasvrfd);
		close(client->pasvlfd);
		// 执行删除用户的时候这两个文件描述符不在fds中，所以不用清除
		client->pasvrfd = -1;
		client->pasvlfd = -1;
	}
	close(fd);
	FD_CLR(fd, &fds);

	struct Client* prev = destroyClient(client);
	if (!prev->next) tail = prev;
	connCnt--;
	return prev;
}

void addClient(int fd) {
	struct Client* newClient = (struct Client*)malloc(sizeof(struct Client));
	initClient(newClient, fd, tail, NULL);
	tail->next = newClient;
	tail = newClient;
	connCnt++;
}

const char* getResponseByCode(int code) {
	return getResponse(code);
}

void initServer() {
	// 初始化用户队列，调用前需要先给head分配内存
	initClient(head, 0, NULL, NULL);
	tail = head;

	tv.tv_sec = 0;
	tv.tv_usec = 0;
}

int enterPassiveMode(int userfd, char* ipAddr, short* port) {
	strcpy(ipAddr, "0.0.0.0");
	*port = 6790;

	struct Client* client = getClientByfd(userfd);
	if (!client) return -1;
	if (client->pasvlfd != -1) {
		// 用户已经请求过PASV，关闭原先的数据端口，开启新的数据端口
		close(client->pasvrfd);
		close(client->pasvlfd);
		client->pasvrfd = -1;
		client->pasvlfd = -1;
	}
	if((client->pasvlfd = setupListen("0.0.0.0", 6790)) != -1) {
		printf(promptDataListen, 6790, client->fd, client->pasvlfd);
		return 1;
	}
	else {
		printf(errorListenFail, 6790);
		return -1;
	}
}

void processClientConn() {
	FD_ZERO(&fds);
	FD_SET(listenfd, &fds);
	
	int maxfd = listenfd;

	struct Client* p = head;
	while (p->next) {
		p = p->next;
		FD_SET(p->fd, &fds);
		if (p->fd > maxfd) maxfd = p->fd;
	}

	int ret = select(maxfd+1, &fds, NULL, NULL, &tv);
	if (ret < 0) {
		printf(errorSelect);
		// return -1;
		return;
	}
	else if (ret == 0) {
		// printf(promptTimeout);
		return;
	}

	p = head;
	while (p->next) {
		p = p->next;
		if (FD_ISSET(p->fd, &fds)) {
			char cmd[5];
			char param[MAXBUF];
			if(receiveFromClient(p->fd, reqBuf, cmd, param) != -1) {
				int res = cmdMapper(p->fd, cmd, param);
				if (res == -1) {
					// 用户连接非正常关闭，服务器层负责删除用户
					printf(errorConnShutDown, p->fd);
					p = deleteClientByfd(p->fd);
				}
				else if (res == -2) {
					printf(promptClientClose, p->fd);
					p = deleteClientByfd(p->fd);
				}
			}
			else {
				// 用户连接非正常关闭，服务器层负责删除用户
				printf(errorConnShutDown, p->fd);
				p = deleteClientByfd(p->fd);
			}
		}
	}

	if (FD_ISSET(listenfd, &fds)) {
		int latestfd;
		if ((latestfd = acceptNewConn(listenfd)) != -1) {
			printf(promptNewConn, latestfd);
			addClient(latestfd);
			// response with 220
			response2Client(latestfd, 220);
		}
		else printf(errorClientConn);
	}
}

void processDataConn() {
	FD_ZERO(&fds);
	FD_SET(listenfd, &fds);

	int maxfd = -1;
	int dataConnCnt = 0;
	struct Client* p = head;
	while (p->next) {
		p = p->next;
		if (p->pasvlfd != -1) {
			FD_SET(p->pasvlfd, &fds);
			dataConnCnt++;
			if (p->pasvlfd > maxfd) maxfd = p->fd;
		}
	}

	if (!dataConnCnt) return;

	int ret = select(maxfd+1, &fds, NULL, NULL, &tv);
	if (ret < 0) {
		printf(errorSelect);
		return;
	}
	else if (ret == 0) {
		// printf(promptTimeout);
		return;
	}

	p = head;
	while (p->next) {
		p = p->next;
		if (p->pasvlfd != -1 && FD_ISSET(p->pasvlfd, &fds)) {
			// 处理客户端数据连接请求
			if ((p->pasvrfd = acceptNewConn(p->pasvlfd)) != -1) {
				printf(promptNewDataConn, p->fd, p->pasvlfd, p->pasvrfd);
			}
			else {
				printf(errorClientDataConn, p->fd, p->pasvlfd);
				continue;
			}
		}
	}
}

int process() {
	head = (struct Client*)malloc(sizeof(struct Client));
	initServer();

	if((listenfd = setupListen("0.0.0.0", 6789)) != -1) printf(promptClientListen, listenPort);
	else {
		printf(errorListenFail, 6789);
		return -1;
	}
	
	while (1) {
		processClientConn();
		processDataConn();
	}
	
	return 0;
}

int main(int argc, char **argv) {
	return process();
}

