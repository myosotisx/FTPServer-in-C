#include "server_cm.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 管理FTP服务器的用户

struct Client* initClient(struct Client* client, int fd, struct Client* prev, struct Client* next) {
	memset(client, 0, sizeof(struct Client));
	memset(client->ipAddr, 0, 32);
	client->fd = fd;
	client->dataListenfd = -1;
	client->dataConnfd = -1;
	client->port = -1;
	client->mode = 0;
	client->tfing = 0;
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

struct Client* getClientByfd(int fd) {
	struct Client* p = getClientHead();
	while (p->next) {
		p = p->next;
		if (p->fd == fd) return p;
	}
	return NULL;
}

int ready2RetriveByfd(int fd) {
	struct Client* p = getClientHead();
	while (p->next) {
		p = p->next;
		if (p->fd == fd) {
			if (p->dataListenfd != -1 && p->dataConnfd != -1) return 1;
			else return 0;
		}
	}
	return 0;
}

int getDataConnfdByfd(int fd) {
	struct Client* p = getClientHead();
	while (p->next) {
		p = p->next;
		if (p->fd == fd) return p->dataConnfd;
	}
	return -1;
}

int getDataListenfdByfd(int fd) {
	struct Client* p = getClientHead();
	while (p->next) {
		p = p->next;
		if (p->fd == fd) return p->dataListenfd;
	}
	return -1;
}

void setClientTransferByfd(int fd, int flag) {
	struct Client* p = getClientHead();
	while (p->next) {
		p = p->next;
		if (p->fd == fd) {
			p->tfing = flag;
			return;
		}
	}
}

void setUsernameByfd(int fd, char* username) {
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

void setDataConnfdByfd(int fd, int sockfd) {
	struct Client* p = getClientHead();
	while (p->next) {
		p = p->next;
		if (p->fd == fd) {
			p->dataConnfd = sockfd;
			return;
		}
	}
}

char* getUsernameByfd(int fd) {
	struct Client* p = getClientHead();
	while (p->next) {
		p = p->next;
		if (p->fd == fd) return p->username;
	}
	return NULL;
}

int getDataModeByfd(int fd) {
	struct Client* p = getClientHead();
	while (p->next) {
		p = p->next;
		if (p->fd == fd) return p->mode;
	}
	return -1;
}

int getIpAddrNPortByfd(int fd, char* ipAddr, int* port) {
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

void setPasswordByfd(int fd, char* password) {
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

void clearDataConnByfd(int fd) {
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

void printClient() {
	struct Client* p = getClientHead();
	while (p->next) {
		p = p->next;
		printf("%d ", p->fd);
	}
	printf("\r\n");
}