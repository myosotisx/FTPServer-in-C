#include "server_util.h"
#include "server_pi.h"
#include "server_cm.h"

#include <sys/socket.h>
#include <netinet/in.h>

#include <unistd.h>
#include <errno.h>

#include <string.h>
#include <memory.h>
#include <stdio.h>

int listenfd;
int latestfd;
int listenPort = 6789;

struct Client* head, *tail;
int connCnt = 0;

char reqBuf[MAXBUF];

struct fd_set fds;
struct timeval tv;

const char promptListen[] = "Server starts listening at port: %d.\r\n";
const char promptTimeout[] = "Select Timeout.\r\n";
const char promptClientClose[] = "Connection with client (fd: %d) closed normally.\r\n";
const char promptReceive[] = "Receive from client (fd: %d):\r\n";
const char promptNewConn[] = "New connection from client (fd: %d).\r\n";

const char errorListenFail[] = "Fail to setup listen! Server aborts!\r\n";
const char errorSelect[] = "Error occurs when using select()! Sever aborts!\r\n";
const char errorFullQueue[] = "Connection queue is full!\r\n";
const char errorInvalidReq[] = "Invalid Request!\r\n";
const char errorConnShutDown[] = "Connection with client (fd: %d) has been shut down!\r\n";

struct Client* getClientHead() {
	return head;
}

struct Client* deleteClientByfd(int fd) {
	close(fd);
	FD_CLR(fd, &fds);
	struct Client* prev = destroyClientByfd(fd);
	if (!prev->next) tail = prev;
	connCnt--;
	return prev;
}

void addClient(int fd) {
	struct Client* newClient = (struct Client*)malloc(sizeof(struct Client));
	initClient(newClient, latestfd, tail, NULL);
	tail->next = newClient;
	tail = newClient;
	connCnt++;
}

int getConnCnt() {
	return connCnt;
}

char* getResponseByCode(int code) {
	return getResponse(code);
}

void initServer() {
	// 初始化用户队列，调用前需要先给head分配内存
	initClient(head, 0, NULL, NULL);
	tail = head;

	tv.tv_sec = 0;
	tv.tv_usec = 0;
}

int process() {
	head = (struct Client*)malloc(sizeof(struct Client));
	initServer();

	if((listenfd = setupListen()) != -1) printf(promptListen, listenPort);
	else {
		printf(errorListenFail);
		return -1;
	}
	
	while (1) {
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
			return -1;
		}
		else if (ret == 0) {
			// printf(promptTimeout);
			continue;
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
			if ((latestfd = acceptNewConn(listenfd)) != -1) {
				printf(promptNewConn, latestfd);
				addClient(latestfd);
				// response with 220
				response2Client(latestfd, 220);
			}
			else continue;
		}
	}
}

int main(int argc, char **argv) {
	return process();
}

