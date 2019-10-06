#ifndef SERVER_UTIL_H
    #define SERVER_UTIL_H
#endif

#include "server_util.h"

struct Client {
	int fd;
	int pasvlfd;
	int pasvrfd;
	int tfing;
	char username[MAXBUF];
	char password[MAXBUF];
	int bytesRecv;
	struct Client* prev;
	struct Client* next;
};