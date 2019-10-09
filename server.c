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
int listenPort = 57301;

struct Client* head, *tail;
int connCnt = 0;

char reqBuf[MAXBUF];

struct fd_set fds;
struct timeval tv;

const char promptClientListen[] = "Server starts listening at port: %d.\r\n";
const char promptDataListen[] = "Server starts data listening at port: %d for client (fd: %d, dataListenfd: %d).\r\n";
const char promptDataConnReady[] = "Server data connection with client (fd: %d, ipAddr: %s, port: %d) is ready.\r\n";
const char promptTimeout[] = "Select Timeout.\r\n";
const char promptClientClose[] = "Connection with client (fd: %d) closed normally.\r\n";
const char promptReceive[] = "Receive from client (fd: %d):\r\n";
const char promptNewConn[] = "New connection from client (fd: %d).\r\n";
const char promptNewDataConn[] = "New data connection from client (fd: %d, dataListenfd: %d, dataConnfd: %d).\r\n";

const char errorListenFail[] = "Fail to setup listen at port: %d!\r\n";
const char errorSelect[] = "Error occurs when using select()! Sever aborts!\r\n";
const char errorClientConn[] = "Connection with client fails!\r\n";
const char errorClientDataConn[] = "Data Connection with client (fd: %d, dataListenfd: %d) fails!\r\n";
const char errorInvalidReq[] = "Invalid Request!\r\n";
const char errorConnShutDown[] = "Connection with client (fd: %d) has been shut down!\r\n";

struct Client* getClientHead() {
	return head;
}

struct Client* deleteClient(int fd) {
	struct Client* client = getClient(fd);
	
	if (!client) return NULL;
	close(client->dataConnfd);
	close(client->dataListenfd);
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

int enterPassiveMode(int userfd, char* ipAddr, int* port) {
	strcpy(ipAddr, "0.0.0.0");
	*port = 57302;

	struct Client* client = getClient(userfd);
	if (!client) return -1;
	if (client->dataListenfd != -1) {
		close(client->dataConnfd);
		close(client->dataListenfd);
		client->dataConnfd = -1;
		client->dataListenfd = -1;
	}
	if((client->dataListenfd = setupListen(ipAddr, *port)) != -1) {
		client->mode = 1;
		printf(promptDataListen, *port, client->fd, client->dataListenfd);
		return 1;
	}
	else {
		printf(errorListenFail, *port);
		return -1;
	}
}

int enterPortMode(int userfd, char* ipAddr, int port) {
	struct Client* client = getClient(userfd);
	if (!client) return -1;
	client->mode = 0;
	memset(client->ipAddr, 0, sizeof(client->ipAddr));
	strcpy(client->ipAddr, ipAddr);
	client->port = port;
	printf(promptDataConnReady, client->fd, client->ipAddr, client->port);
	return 1;
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
			char cmd[MAXBUF];
			char param[MAXBUF];
			if(receiveFromClient(p->fd, reqBuf, cmd, param) != -1) {
				int res = cmdMapper(p->fd, cmd, param);
				if (res == -1) {
					// 用户连接非正常关闭，服务器层负责删除用户
					printf(errorConnShutDown, p->fd);
					p = deleteClient(p->fd);
				}
				else if (res == -2) {
					printf(promptClientClose, p->fd);
					p = deleteClient(p->fd);
				}
			}
			else {
				// 用户连接非正常关闭，服务器层负责删除用户
				printf(errorConnShutDown, p->fd);
				p = deleteClient(p->fd);
			}
		}
	}

	if (FD_ISSET(listenfd, &fds)) {
		int latestfd;
		if ((latestfd = acceptNewConn(listenfd)) != -1) {
			printf(promptNewConn, latestfd);
			addClient(latestfd);
			// response with 220
			response(latestfd, 220);
		}
		else printf(errorClientConn);
	}
}

void processDataConn() {
	FD_ZERO(&fds);

	int maxfd = 0;
	struct Client* p = head;
	while (p->next) {
		p = p->next;
		if (p->dataListenfd != -1) {
			FD_SET(p->dataListenfd, &fds);
			if (p->dataListenfd > maxfd) maxfd = p->dataListenfd;
		}
	}

	if (!maxfd) return;

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
		if (p->dataListenfd != -1 && FD_ISSET(p->dataListenfd, &fds)) {
			// 处理客户端数据连接请求
			if ((p->dataConnfd = acceptNewConn(p->dataListenfd)) != -1) {
				printf(promptNewDataConn, p->fd, p->dataListenfd, p->dataConnfd);
			}
			else {
				printf(errorClientDataConn, p->fd, p->dataListenfd);
				continue;
			}
		}
	}
}

int process() {
	head = (struct Client*)malloc(sizeof(struct Client));
	initServer();

	if((listenfd = setupListen("0.0.0.0", listenPort)) != -1) printf(promptClientListen, listenPort);
	else {
		printf(errorListenFail, listenPort);
		return -1;
	}
	
	while (1) {
		processDataConn();
		processClientConn();
	}
	
	return 0;
}

int main(int argc, char **argv) {
	return process();
}

