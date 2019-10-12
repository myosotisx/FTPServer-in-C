#include "server_util.h"
#include "server_pi.h"
#include "server_cm.h"
#include "server_info.h"

#include <unistd.h>
#include <string.h>
#include <stdlib.h>

int listenfd;
int listenPort = 21;
int connCnt = 0;
char reqBuf[MAXBUF];
fd_set fds;
struct timeval tv;
struct Client* head, *tail;

struct Client* getClientHead() {
	return head;
}

const char* getResponseByCode(int code) {
	return getResponse(code);
}

int enterPassiveMode(int fd, char* ipAddr, int* port) {
	closeDataConn(fd);

	strcpy(ipAddr, "0.0.0.0");
	*port = fd+57300;
	int dataListenfd;

	if((dataListenfd = setupListen(ipAddr, *port, 1)) != -1) {
		if (setDataMode(fd, 1) != -1 && setDataListenfd(fd, dataListenfd) != -1) {
			printf(promptDataListen, *port, fd, dataListenfd);
			return 1;
		}
		else {
			printf(errorClientNotFound, fd);
			close(dataListenfd);
			return -1;
		}
	}
	else {
		printf(errorListenFail, *port);
		return -1;
	}
}

int enterPortMode(int fd, char* ipAddr, int port) {
	closeDataConn(fd);

	if (setDataMode(fd, 0) != -1 && setIpAddrNPort(fd, ipAddr, port) != -1) {
		printf(promptDataConnReady, fd, ipAddr, port);
		return 1;
	}
	else {
		printf(errorClientNotFound, fd);
		return -1;
	}
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

void initServer() {
	// 初始化用户队列，调用前需要先给head分配内存
	initClient(head, 0, NULL, NULL);
	tail = head;

	tv.tv_sec = 0;
	tv.tv_usec = 0;
}

void checkClient() {
	struct Client* p = head;
	while (p->next) {
		p = p->next;
		
		if (p->state == ERRORQUIT) {
			printf(errorConnShutDown, p->fd);
			p = deleteClient(p->fd);
		}
		else if (p->state == NORMQUIT) {
			printf(promptClientClose, p->fd);
			p = deleteClient(p->fd);
		}
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
			char cmd[MAXCMD];
			char param[MAXPARAM];
			if(receive(p->fd, reqBuf, cmd, param) != -1) {
				cmdMapper(p->fd, cmd, param);
			}
			else {
				setClientState(p->fd, ERRORQUIT);
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

	if((listenfd = setupListen("0.0.0.0", listenPort, 1)) != -1) printf(promptClientListen, listenPort);
	else {
		printf(errorListenFail, listenPort);
		return -1;
	}
	
	while (1) {
		checkClient();
		processDataConn();
		processClientConn();
	}
	
	return 0;
}

int main(int argc, char **argv) {
	return process();
}

