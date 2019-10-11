#ifndef SERVER_CM_H
#define SERVER_CM_H

#include "server_util.h"

enum State { ERRORQUIT = -1, NORMQUIT = 0, NORM = 1, TRANSFER = 2, WAITUSER = 3, WAITPASS = 4, WAITRNTO = 5 };

struct Client {
	enum State state;
	int fd;
	int dataListenfd;
	int dataConnfd;
	int port;
	int mode;  // 0表示PORT，1表示PASV，默认为0
	char ipAddr[32];
	char username[32];
	char password[32];
	char workDir[MAXPATH];
	char reserved[10][MAXPATH];
	void* reservedPtr[10];
	int bytesRecv;
	struct Client* prev;
	struct Client* next;
};

#endif