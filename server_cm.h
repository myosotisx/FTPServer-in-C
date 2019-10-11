#ifndef SERVER_UTIL_H
#define SERVER_UTIL_H
#endif

#include "server_util.h"

struct Client {
	int fd;
	int dataListenfd;
	int dataConnfd;
	int tfing;
	int port;
	int mode;  // 0表示PORT，1表示PASV，默认为0
	char ipAddr[32];
	char username[MAXBUF];
	char password[MAXBUF];
	char workDir[MAXPATH];
	char reserved[10][MAXPATH];
	int bytesRecv;
	struct Client* prev;
	struct Client* next;
	
};