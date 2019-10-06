#include "server_cm.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 管理FTP服务器的用户

struct Client* initClient(struct Client* client, int fd, struct Client* prev, struct Client* next) {
	memset(client, 0, sizeof(struct Client));
	client->fd = fd;
	client->pasvfd = -1;
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

char* getUsernameByfd(int fd) {
	struct Client* p = getClientHead();
	while (p->next) {
		p = p->next;
		if (p->fd == fd) return p->username;
	}
	return NULL;
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

void printClient() {
	struct Client* p = getClientHead();
	while (p->next) {
		p = p->next;
		printf("%d ", p->fd);
	}
	printf("\r\n");
}