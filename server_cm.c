#include "server_cm.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 管理FTP服务器的用户

struct Client* initClient(struct Client* client, int fd, struct Client* prev, struct Client* next) {
	memset(client, 0, sizeof(struct Client));
	memset(client->ipAddr, 0, 32);
	memset(client->workDir, 0, MAXPATH);
	strcpy(client->workDir, "/");
	client->state = WAITUSER;
	client->fd = fd;
	client->dataListenfd = -1;
	client->dataConnfd = -1;
	client->port = -1;
	client->mode = 0;
	client->bytesRecv = 0;
	client->transThread = 0;
	client->prev = prev;
	client->next = next;
	return client;
}

struct Client* destroyClient(struct Client* client) {
	// 销毁client，返回client前一个用户
	struct Client* p = client->prev;
	p->next = client->next;
	if (client->next) client->next->prev = p;
	free(client);
	return p;
}

struct Client* destroyClientByfd(int fd) {
	// 根据文件描述符查找用户，销毁后返回前一个用户
	struct Client* p = getClientHead();
	while (p->next) {
		p = p->next;
		if (p->fd == fd) return destroyClient(p);
	}
	return NULL;
}

struct Client* getClient(int fd) {
	struct Client* p = getClientHead();
	while (p->next) {
		p = p->next;
		if (p->fd == fd) return p;
	}
	return NULL;
}

int getClientState(int fd) {
	struct Client* p = getClient(fd);
	if (p) return p->state;
	else return ERRORQUIT;
}

int setClientState(int fd, int state) {
	struct Client* p = getClient(fd);
	if (p) {
		p->state = state;
		printf("Debug Info: Client (fd: %d) state set to %d\r\n", p->fd, p->state);
		return 1;
	}
	else return -1;
}

int getDataMode(int fd) {
	struct Client* p = getClient(fd);
	if (p) return p->mode;
	else return -1;
}

int setDataMode(int fd, int mode) {
	struct Client* p = getClient(fd);
	if (p) {
		p->mode = mode;
		return 1;
	}
	else return -1;
}

int getDataConnfd(int fd) {
	struct Client* p = getClient(fd);
	if (p) return p->dataConnfd;
	else return -1;
}

int setDataConnfd(int fd, int dataConnfd) {
	struct Client* p = getClient(fd);
	if (p) {
		p->dataConnfd = dataConnfd;
		return 1;
	}
	else return -1;
}

int getDataListenfd(int fd) {
	struct Client* p = getClient(fd);
	if (p) return p->dataListenfd;
	else return -1;
}

int setDataListenfd(int fd, int dataListenfd) {
	struct Client* p = getClient(fd);
	if (p) {
		p->dataListenfd = dataListenfd;
		return 1;
	}
	else return -1;
}

int clearDataConn(int fd) {
	struct Client* p = getClient(fd);
	if (p) {
		memset(p->ipAddr, 0, 32);
		p->port = -1;
		p->dataListenfd = -1;
		p->dataConnfd = -1;
		
		p->mode = 0;  // 0表示PORT，1表示PASV，默认为0
		return 1;
	}
	else return -1;
}

const char* getUsername(int fd) {
	struct Client* p = getClient(fd);
	if (p) return p->username;
	else return NULL;
}

int setUsername(int fd, const char* username) {
	struct Client* p = getClient(fd);
	if (p) {
		memset(p->username, 0, 32);
		strcpy(p->username, username);
		return 1;
	}
	else return -1;
}

const char* getWorkDir(int fd) {
	struct Client* p = getClient(fd);
	if (p) return p->workDir;
	else return NULL;
}

int setWorkDir(int fd, char* path) {
	struct Client* p = getClient(fd);
	if (p) {
		memset(p->workDir, 0, MAXPATH);
		strcpy(p->workDir, path);
		return 1;
	}
	else return -1;
}

int getIpAddrNPort(int fd, char* ipAddr, int* port) {
	struct Client* p = getClient(fd);
	if (p) {
		memset(ipAddr, 0, 32);
		strcpy(ipAddr, p->ipAddr);
		*port = p->port;
		return 1;
	}
	else return -1;
}

int setIpAddrNPort(int fd, const char* ipAddr, int port) {
	struct Client* p = getClient(fd);
	if (p) {
		memset(p->ipAddr, 0, 32);
		strcpy(p->ipAddr, ipAddr);
		p->port = port;
		return 1;
	}
	else return -1;
}

int setPassword(int fd, const char* password) {
	struct Client* p = getClient(fd);
	if (p) {
		memset(p->password, 0, 32);
		strcpy(p->password, password);
		return 1;
	}
	else return -1;
}

pthread_t getTransThread(int fd) {
	struct Client* p = getClient(fd);
	if (p) return p->transThread;
	else return 0;
}

int setTransThread(int fd, pthread_t transThread) {
	struct Client* p = getClient(fd);
	if (p) {
		p->transThread = transThread;
		printf("Debug Info in CM: client (fd: %d) transThread set to %lu\r\n", fd, transThread);
		return 1;
	}
	else return -1;
}

const char* getReserved(int fd, int index) {
	struct Client* p = getClient(fd);
	if (p) return p->reserved[index];
	else return NULL;
}

int setReserved(int fd, int index, const char* content) {
	struct Client* p = getClient(fd);
	if (p) {
		memset(p->reserved[index], 0, MAXPATH);
		strcpy(p->reserved[index], content);
		return 1;
	}
	else return -1;
}

void* getReservedPtr(int fd, int index) {
	struct Client* p = getClient(fd);
	if (p) return p->reservedPtr[index];
	else return NULL;
}

int setReservedPtr(int fd, int index, void* ptr) {
	struct Client* p = getClient(fd);
	if (p) {
		p->reservedPtr[index] = ptr;
		return 1;
	}
	else return -1;
}
