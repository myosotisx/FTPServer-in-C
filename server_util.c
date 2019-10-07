#include "server_util.h"

#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <unistd.h>
#include <errno.h>

#include <ctype.h>
#include <string.h>
#include <memory.h>
#include <stdlib.h>
#include <stdio.h>

int readBuf(int sockfd, void* buf) {
	int readLen;
	int p = 0, bufp = 0;
	while (!bufp) {
		readLen = read(sockfd, buf+bufp, MAXBUF-1-bufp);
		if (readLen < 0) {
			// 传输错误或关闭连接
			printf("Error read(): %s(%d)\n", strerror(errno), errno);
			// close(sockfd);
			return -1;
		}
		else if (readLen == 0) {
			// close(sockfd);
			return 0;
		}
		else {
			p += readLen;
			bufp = readLen % (MAXBUF-1);
		}
	}
	return p;
}

int writeBuf(int sockfd, const void* buf, int len) {
	// printf("Debug Info: write to fd: %d\r\n", sockfd);
	int writeLen;
	int p = 0;
	while (p < len) {
		writeLen = write(sockfd, buf, len);
		if (writeLen < 0) {
			printf("Error write(): %s(%d)\n", strerror(errno), errno);
			// close(sockfd);
			return -1;
 		} 
		else p += writeLen;
	}
	return p;
}

int writeFile(int fd, FILE* file) {
	unsigned char fileBuf[MAXBUF];
	int len;
	int dataConnfd = getDataConnfdByfd(fd);
	while ((len = fread(fileBuf, sizeof(unsigned char), MAXBUF, file))) {
		if (writeBuf(dataConnfd, fileBuf, MAXBUF) == -1) return -1;
	}
	return 1;
}

unsigned int getFileSize(FILE* file) {
	unsigned int size;
	fseek(file, 0, SEEK_END);
	size = ftell(file);
	fseek(file, 0, SEEK_SET);
	return size;
}

int setupListen(char* ipAddr, short port) {
	struct sockaddr_in addr;
	int listenfd;

	if ((listenfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
		printf("Error socket(): %s(%d)\n", strerror(errno), errno);
		return -1;
	}

	printf("listenfd: %d\r\n", listenfd);

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = /*inet_addr(ipAddr);*/htonl(INADDR_ANY);

	if (bind(listenfd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
		printf("Error bind(): %s(%d)\n", strerror(errno), errno);
		return -1;
	}

	if (listen(listenfd, BACKLOG) == -1) {
		printf("Error listen(): %s(%d)\n", strerror(errno), errno);
		return -1;
	}
	return listenfd;
}

void closeListen(int fd) {
	close(fd);
}

void getCmdNParam(char* request, char* cmd, char* param) {
	memset(cmd, 0, MAXBUF);
	memset(param, 0, MAXBUF);
	int reqLen = strlen(request);
	char* p = strchr(request, ' ');
	// seperate cmd and param
	if (!p) {
		// int q = reqLen-2 <= 4 ? reqLen-2 : 4;
		int q = reqLen-2;
		strncpy(cmd, request, q);
	}
	else {
		// int q = p-request <= 4 ? p-request : 4;
		int q = p-request;
		strncpy(cmd, request, q);
		strncpy(param, request+q+1, reqLen-(q+1)-1); // 处理部分客户端请求以\n结尾
		int paramLen = strlen(param);
		if (param[paramLen-1] == '\r') param[paramLen-1] = 0;
	}
}

int response2Client(int fd, int code) {
	const char* response = getResponseByCode(code);
	return writeBuf(fd, response, strlen(response));
}

int receiveFromClient(int fd, char* reqBuf, char* cmd, char* param) {
	int len = readBuf(fd, reqBuf);
	printf("Debug Info: New receive from client (fd: %d). Receive length is %d.\r\n", fd, len);
	if (len > 0) {
		reqBuf[len] = 0;
		printf("%s", reqBuf);
		getCmdNParam(reqBuf, cmd, param);
		return 1;
	}
	else return -1;
}

int acceptNewConn(int listenfd) {
	int newfd;
	if ((newfd = accept(listenfd, NULL, NULL)) == -1) {
		printf("Error accept(): %s(%d)\n", strerror(errno), errno);
		return -1;
	}
	// 暂时不限制连接数量
	/*if (getConnCnt() < BACKLOG) {
		return newfd;
	}
	else {
		// write something to client
		close(newfd);
		return -1;
	}*/
	else return newfd;
}

int setupDataConnByfd(int fd) {
	char ipAddr[32];
	int port;
	memset(ipAddr, 0, 32);
	if (getIpAddrNPortByfd(fd, ipAddr, &port) == -1) return -1;

	printf("ipAddr: %s port: %d\r\n", ipAddr, port);

	struct sockaddr_in addr, addrClient;
	int sockfd;

	if ((sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
		printf("Error socket(): %s(%d)\n", strerror(errno), errno);
		return -1;
	}

	printf("sockfd: %d\r\n", sockfd);

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(57300);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
		printf("Error bind(): %s(%d)\n", strerror(errno), errno);
		// 如果绑定失败说明已经建立过PORT数据连接，重新连接即可
		// return -1;
	}

	memset(&addr, 0, sizeof(addrClient));
	addrClient.sin_family = AF_INET;
	addrClient.sin_port = htons(port);
	if (inet_pton(AF_INET, ipAddr, &addrClient.sin_addr) <= 0) {
		printf("Error inet_pton(): %s(%d)\n", strerror(errno), errno);
		return -1;
	}

	if (connect(sockfd, (struct sockaddr*)&addrClient, sizeof(addrClient)) < 0) {
		printf("Error connect(): %s(%d)\n", strerror(errno), errno);
		return -1;
	}

	setDataConnfdByfd(fd, sockfd);

	return 1;
}

void closeDataConnByfd(int fd) {
	int dataConnfd;
	if ((dataConnfd = getDataConnfdByfd(fd)) != -1) close(dataConnfd);
	clearDataConnByfd(fd);
}

void strReplace(char* str, char oldc, char newc) {
	int len = strlen(str);
	for (int i = 0;i < len;i++) {
		if (str[i] == oldc) str[i] = newc;
	}
}

void parseIpAddrNPort(char* param, char* ipAddr, int* port) {
	char p1[8];
	char p2[8];
	memset(p1, 0, sizeof(p1));
	memset(p2, 0, sizeof(p2));
	memset(ipAddr, 0, 32);
	int rfirst = -1;
	int i = strlen(param)-1;
	while (i >= 0) {
		if (param[i] == ',') {
			if (rfirst == -1) {
				rfirst = i;
				strncpy(p2, param+rfirst+1, strlen(param)-1-rfirst);
			}
			else {
				strncpy(p1, param+i+1, rfirst-i-1);
				strncpy(ipAddr, param, i);
				strReplace(ipAddr, ',', '.');
				*port = atoi(p1)*256+atoi(p2);
				return;
			}
		}
		i--;
	}
	
}