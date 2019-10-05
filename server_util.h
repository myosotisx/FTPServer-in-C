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

void setUsernameByfd(int fd, char* username);

char* getUsernameByfd(int fd);

void setPasswordByfd(int fd, char* password);

void printClient();

///////////////////////////////////////////////////////////////////////////////////////////////////////

// 服务器层实现

struct Client* getClientHead();

int getConnCnt();

char* getResponseByCode(int code);

///////////////////////////////////////////////////////////////////////////////////////////////////////

int readBuf(int sockfd, void* buf);

int writeBuf(int sockfd, void* buf, int len);

int setupListen();

void getCmdNParam(char* request, char* cmd, char* param);

int response2Client(int fd, int code);

int receiveFromClient(int fd, char* reqBuf, char* cmd, char* param);

int acceptNewConn(int listenfd);




