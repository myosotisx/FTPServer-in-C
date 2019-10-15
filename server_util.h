#ifndef SERVER_UTIL_H
#define SERVER_UTIL_H

#include <stdio.h>
#include <pthread.h>

#define __OSX__
#define __DEBUG__

#define BACKLOG 10
#define MAXBUF 8192
#define MAXPATH 1024
#define MAXLINE 4096
#define MAXCMD 1024
#define MAXPARAM 4096
#define MAXRES 1024

///////////////////////////////////////////////////////////////////////////////////////////////////////

// 允许开发者自定义用户信息

enum State;

struct Client;

struct Client* initClient(struct Client* client, int fd, struct Client* prev, struct Client* next);

struct Client* destroyClient(struct Client* client);

struct Client* destroyClientByfd(int fd);

struct Client* getClient(int fd);

int getClientState(int fd);

int setClientState(int fd, int state);

int getDataMode(int fd);

int setDataMode(int fd, int mode);

int getDataConnfd(int fd);

int setDataConnfd(int fd, int dataConnfd);

int getDataListenfd(int fd);

int setDataListenfd(int fd, int dataListenfd);

int clearDataConn(int fd);

const char* getUsername(int fd);

int setUsername(int fd, const char* username);

const char* getWorkDir(int fd);

int setWorkDir(int fd, char* path);

int getIpAddrNPort(int fd, char* ipAddr, int* port);

int setIpAddrNPort(int fd, const char* ipAddr, int port);

int setPassword(int fd, const char* password);

pthread_t getTransThread(int fd);

int setTransThread(int fd, pthread_t transThread);

const char* getReserved(int fd, int index);

int setReserved(int fd, int index, const char* content);

void* getReservedPtr(int fd, int index);

int setReservedPtr(int fd, int index, void* ptr);

///////////////////////////////////////////////////////////////////////////////////////////////////////

// 服务器层实现

struct Client* getClientHead();

const char* getResponseByCode(int code);

int enterPassiveMode(int fd, char* ipAddr, int* port);

int enterPortMode(int fd, char* ipAddr, int port);

///////////////////////////////////////////////////////////////////////////////////////////////////////

// 字符串操作工具

void strReplace(char* str, char oldc, char newc);

void getCmdNParam(char* request, char* cmd, char* param);

void parseIpAddrNPort(char* param, char* ipAddr, int* port);

char* getFormatPath(char* formatPath, const char* path);

// 文件处理工具

const char* getRootPath();

int setRootPath(const char* nRootPath);

int isFile(const char* path);

int isDir(const char* path);

long long getFileSize(FILE* file);

int copyFile(const char* oPath, const char* nPath);

int removeAll(const char* path);

char* listDir(char* fileList, const char* path, const char* param);

// socket工具

int readBuf(int sockfd, void* buf);

int writeBuf(int sockfd, const void* buf, int len);

int receive(int fd, char* reqBuf, char* cmd, char* param);

int response(int fd, int code);

int setupListen(char* ipAddr, int port, int opt);

int acceptNewConn(int listenfd);

int setupDataConn(int fd, int opt);

void closeDataConn(int fd);

// 处理Client请求工具

char* getFilePath(int fd, char* path, char* fileName);

char* getClientAbsPath(int fd, char* cAbsPath, const char* path);

char* getServerRelPath(int fd, char* sRelPath, const char* path);

int makeDir(int fd, const char* path);

int changeWorkDir(int fd, const char* path);

int removeDir(int fd, const char* path);

int setFile2Rename(int fd, const char* path);

int renameFile(int fd, const char* oPath, const char* nPath);

char* getFileList(int fd, char* fileList, const char* path);

#endif