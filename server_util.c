#include "server_util.h"

#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netinet/in.h>

#include <unistd.h>
#include <dirent.h>
#include <errno.h>

#include <ctype.h>
#include <string.h>
#include <memory.h>
#include <stdlib.h>
#include <stdio.h>

char rootPath[MAXPATH] = "FTPFile";
char userPath[MAXPATH] = "/";

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
	int dataConnfd = getDataConnfd(fd);
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

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = inet_addr(ipAddr);

	if (bind(listenfd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
		printf("Error bind(): %s(%d)\n", strerror(errno), errno);
		// 如果绑定失败说明已经建立过PASV数据连接，重新开启监听即可
		// return -1; 
	}

	if (listen(listenfd, BACKLOG) == -1) {
		printf("Error listen(): %s(%d)\n", strerror(errno), errno);
		return -1;
	}
	return listenfd;
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

int response(int fd, int code) {
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

int setupDataConn(int fd) {
	char ipAddr[32];
	int port;
	memset(ipAddr, 0, 32);
	if (getIpAddrNPort(fd, ipAddr, &port) == -1) return -1;

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

	setDataConnfd(fd, sockfd);

	return 1;
}

void closeDataConn(int fd) {
	int dataConnfd, dataListenfd;
	close(getDataListenfd(fd));
	close(getDataConnfd(fd));
	clearDataConn(fd);
}

char* getFilePath(int fd, char* path, char* fileName) {
	memset(path, 0, MAXPATH);
	strcpy(path, rootPath);
	strcat(path, getWorkDir(fd));
	strcat(path, "/");
	strcat(path, fileName);
	return path;
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

char* getFormatPath(char* formatPath, const char* path) {
	memset(formatPath, 0, MAXPATH);
	int p = 0;
	int len = strlen(path);
	for (int i = 0;i < len;i++) {
		if (path[i] == '\"') {
			formatPath[p++] = '\"';
			formatPath[p++] = '\"';
		}
		else if (path[i] == '\n');
		else formatPath[p++] = path[i];
	}
	return formatPath;
}

int makeDir(int fd, const char* path) {
	char realPath[MAXPATH];
	memset(realPath, 0, MAXPATH);
	strcpy(realPath, rootPath);
	if (path[0] == '/') {
		strcat(realPath, path);
	}
	else {
		strcat(realPath, getWorkDir(fd));
		if (realPath[strlen(realPath)-1] != '/') strcat(realPath, "/");
		strcat(realPath, path);
	}
	return mkdir(realPath, S_IRWXU|S_IRWXG|S_IROTH|S_IXOTH);
}

int changeWorkDir(int fd, const char* path) {
	char nWorkDir[MAXPATH];
	char dirName[MAXPATH];
	memset(nWorkDir, 0, MAXPATH);
	// int rsep = 0;
	// int t = 0;
	int p, q;
	if (path[0] == '/') {
		// 绝对路径
		p = 1, q = 1;
		strcat(nWorkDir, "/");
	}
	else {
		p = 0, q = 0;
		strcat(nWorkDir, getWorkDir(fd));
	}
	int rsep = strrchr(nWorkDir, '/')-nWorkDir;
	int t = strlen(nWorkDir)-1;
	int len = strlen(path);
	while (q < len) {
		if (path[q] == '/') {
			memset(dirName, 0, MAXPATH);
			strncpy(dirName, path+p, q-p);
			// printf("dirName: %s\r\n", dirName);
			if (!strcmp(dirName, "..")) {
				if (t) {
					t = rsep-1 > 0 ? rsep-1 : 0;
					nWorkDir[t+1] = 0;
					rsep = strrchr(nWorkDir, '/')-nWorkDir;
				}
			}
			else if (strcmp(dirName, "")) {
				if (t) {
					strcat(nWorkDir, "/");
					rsep = t+1;
				}
				strcat(nWorkDir, dirName);
				t = strlen(nWorkDir)-1;
			}
			p = q+1;
			// printf("nWorkDir: %s\r\n", nWorkDir);
		}
		q++;
	}
	if (path[len-1] != '/') {
		memset(dirName, 0, MAXPATH);
		strncpy(dirName, path+p, len-p);
		if (!strcmp(dirName, "..")) {
			if (t) {
				t = rsep-1 > 0 ? rsep-1 : 0;
				nWorkDir[t+1] = 0;	
			}
		}
		else {
			if (t) strcat(nWorkDir, "/");
			strcat(nWorkDir, dirName);
		}
	}
	
	
	char realPath[MAXPATH];
	memset(realPath, 0, MAXPATH);
	strcpy(realPath, rootPath);
	strcat(realPath, nWorkDir);
	if (opendir(realPath) && setWorkDir(fd, nWorkDir) != -1) return 1;
	else return -1;
}