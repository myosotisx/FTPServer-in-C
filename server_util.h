#ifndef SERVER_UTIL_H
    #define SERVER_UTIL_H
#endif

#define BACKLOG 10
#define MAXBUF 8192

///////////////////////////////////////////////////////////////////////////////////////////////////////

// 允许服务器自定义用户信息

struct Client;

struct Client* initClient(struct Client* client, int fd, struct Client* prev, struct Client* next);

struct Client* destroyClient(struct Client* client);

struct Client* destroyClientByfd(int fd);

struct Client* getClientByfd(int fd);

void setUsernameByfd(int fd, char* username);

char* getUsernameByfd(int fd);

void setPasswordByfd(int fd, char* password);

void printClient();

///////////////////////////////////////////////////////////////////////////////////////////////////////

// 服务器层实现

struct Client* getClientHead();

const char* getResponseByCode(int code);

int enterPassiveMode(int userfd, char* ipAddr, short* port);

///////////////////////////////////////////////////////////////////////////////////////////////////////

int readBuf(int sockfd, void* buf);

int writeBuf(int sockfd, const void* buf, int len);

int setupListen(char* ipAddr, short port);

void closeListen(int fd);

void getCmdNParam(char* request, char* cmd, char* param);

int response2Client(int fd, int code);

int receiveFromClient(int fd, char* reqBuf, char* cmd, char* param);

void strReplace(char* str, char oldc, char newc);

int acceptNewConn(int listenfd);






