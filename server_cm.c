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
	client->state = WAITUSER; // 暂时设置NORM
	client->fd = fd;
	client->dataListenfd = -1;
	client->dataConnfd = -1;
	client->port = -1;
	client->mode = 0;
	client->bytesRecv = 0;
	client->prev = prev;
	client->next = next;
	return client;
}

struct Client* destroyClient(struct Client* client) {
	// 销毁client，返回client前一个用户
	struct Client* p = client->prev;
	p->next = client->next;
	if (client->next) client->next->prev = p;
	// else tail = p;
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

int getDataConnfd(int fd) {
	struct Client* p = getClientHead();
	while (p->next) {
		p = p->next;
		if (p->fd == fd) return p->dataConnfd;
	}
	return -1;
}

int getDataListenfd(int fd) {
	struct Client* p = getClientHead();
	while (p->next) {
		p = p->next;
		if (p->fd == fd) return p->dataListenfd;
	}
	return -1;
}

void setUsername(int fd, char* username) {
	struct Client* p = getClientHead();
	while (p->next) {
		p = p->next;
		if (p->fd == fd) {
			memset(p->username, 0, sizeof(p->username));
			strcpy(p->username, username);
			return;
		}
	}
}

void setDataConnfd(int fd, int sockfd) {
	struct Client* p = getClientHead();
	while (p->next) {
		p = p->next;
		if (p->fd == fd) {
			p->dataConnfd = sockfd;
			return;
		}
	}
}

char* getUsername(int fd) {
	struct Client* p = getClientHead();
	while (p->next) {
		p = p->next;
		if (p->fd == fd) return p->username;
	}
	return NULL;
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

int getDataMode(int fd) {
	struct Client* p = getClientHead();
	while (p->next) {
		p = p->next;
		if (p->fd == fd) return p->mode;
	}
	return -1;
}

int getIpAddrNPort(int fd, char* ipAddr, int* port) {
	struct Client* p = getClientHead();
	while (p->next) {
		p = p->next;
		if (p->fd == fd) {
			strcpy(ipAddr, p->ipAddr);
			*port = p->port;
			return 1;
		}
	}
	return -1;
}

void setPassword(int fd, char* password) {
	struct Client* p = getClientHead();
	while (p->next) {
		p = p->next;
		if (p->fd == fd) {
			memset(p->password, 0, sizeof(p->password));
			strcpy(p->password, password);
			return;
		}
	}
}

int setReserved(int fd, int index, const char* content) {
	struct Client* client = getClient(fd);
	if (client) {
		memset(client->reserved[index], 0, MAXPATH);
		strcpy(client->reserved[index], content);
		return 1;
	}
	else return -1;
}

int setReservedPtr(int fd, int index, void* ptr) {
	struct Client* client = getClient(fd);
	if (client) {
		client->reservedPtr[index] = ptr;
		return 1;
	}
	else return -1;
}

const char* getReserved(int fd, int index) {
	struct Client* client = getClient(fd);
	if (client) return client->reserved[index];
	else return NULL;
}

void* getReservedPtr(int fd, int index) {
	struct Client* client = getClient(fd);
	if (client) return client->reservedPtr[index];
	else return NULL;
}

void clearDataConn(int fd) {
	struct Client* p = getClientHead();
	while (p->next) {
		p = p->next;
		if (p->fd == fd) {
			memset(p->ipAddr, 0, 32);
			p->port = -1;
			p->dataListenfd = -1;
			p->dataConnfd = -1;
			
			p->mode = 0;  // 0表示PORT，1表示PASV，默认为0
			return;
		}
	}
}

void setClientState(int fd, int state) {
	struct Client* client = getClient(fd);
	if (client) {
		client->state = state;
		printf("Client (fd: %d) state set to %d\r\n", client->fd, client->state);
	}
}

int getClientState(int fd) {
	struct Client* client = getClient(fd);
	if (client) return client->state;
	else return ERRORQUIT;
}

void printClient() {
	struct Client* p = getClientHead();
	while (p->next) {
		p = p->next;
		printf("%d ", p->fd);
	}
	printf("\r\n");
}