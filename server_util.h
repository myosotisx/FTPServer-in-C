#ifndef SERVER_UTIL_H
    #define SERVER_UTIL_H
#endif

#include <stdio.h>

#define BACKLOG 10
#define MAXBUF 8192
#define MAXPATH 1024

///////////////////////////////////////////////////////////////////////////////////////////////////////

// 允许服务器自定义用户信息

struct Client;

struct Client* initClient(struct Client* client, int fd, struct Client* prev, struct Client* next);

struct Client* destroyClient(struct Client* client);

struct Client* destroyClientByfd(int fd);

struct Client* getClientByfd(int fd);

int ready2RetriveByfd(int fd);

int getDataConnfdByfd(int fd);

int getDataListenfdByfd(int fd);

int getIpAddrNPortByfd(int fd, char* ipAddr, int* port);

void setClientTransferByfd(int fd, int flag);

void setDataConnfdByfd(int fd, int sockfd);

void setUsernameByfd(int fd, char* username);

void clearDataConnByfd(int fd);

char* getUsernameByfd(int fd);

int getDataModeByfd(int fd);

void setPasswordByfd(int fd, char* password);

void printClient();

///////////////////////////////////////////////////////////////////////////////////////////////////////

// 服务器层实现

struct Client* getClientHead();

const char* getResponseByCode(int code);

int enterPassiveMode(int userfd, char* ipAddr, int* port);

int enterPortMode(int userfd, char* ipAddr, int port);

///////////////////////////////////////////////////////////////////////////////////////////////////////

int readBuf(int sockfd, void* buf);

int writeBuf(int sockfd, const void* buf, int len);

int writeFile(int fd, FILE* file);

unsigned int getFileSize(FILE* file);

int setupListen(char* ipAddr, short port);

void getCmdNParam(char* request, char* cmd, char* param);

void getFilePath(char* path, char* param);

int response2Client(int fd, int code);

int receiveFromClient(int fd, char* reqBuf, char* cmd, char* param);

void strReplace(char* str, char oldc, char newc);

void parseIpAddrNPort(char* param, char* ipAddr, int* port);

int acceptNewConn(int listenfd);

int setupDataConnByfd(int fd);

void closeDataConnByfd(int fd);






