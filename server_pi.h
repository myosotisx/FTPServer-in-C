#ifndef SERVER_PI_H
#define SERVER_PI_H

const char* getResponse(int code);

int validCmd(char* cmd);

int cmdMapper(int fd, char* cmd, char* param);

#endif