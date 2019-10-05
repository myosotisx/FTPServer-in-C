#include "server_pi.h"
#include "server_util.h"

#include <string.h>

const char response220[] = "220 FTP server ready.\r\n";
const char response221[] = "221-Thanks for using this FTP service.\r\n221 Goodbye.\r\n";
const char response230[] = "230-\r\n230-Welcome to stx's FTP server!\r\n230-You can download what ever you want here.\r\n230-\r\n230 Guest login ok, access restrictions apply.\r\n";
const char response331[] = "331 Please send your complete e-mail address as password.\r\n";
const char response332[] = "332 Please send your username to log in (only support \"anonymous\" now).\r\n";
const char response500[] = "500 Syntax error!\r\n";
const char response504[] = "504 Parameters not supported!\r\n";
const char response530[] = "530 Username is unacceptable (only support \"anonymous\" now)!\r\n";

char* getResponse(int code) {
    switch (code) {
        case 220: return response220;
        case 221: return response221;
        case 230: return response230;
        case 331: return response331;
        case 332: return response332;
        case 500: return response500;
        case 504: return response504;
        case 530: return response530;
        default: return response500;
    }
}

int handleUSER(int fd, char* param) {
    setUsernameByfd(fd, param);
	// response with 331
	return response2Client(fd, 331);
}

int handlePASS(int fd, char* param) {
    // check username
    if (!strcmp(getUsernameByfd(fd), "anonymous")) {
        setPasswordByfd(fd, param);
        // response with 230
	    return response2Client(fd, 230);
    }
    else {
        // response with 530
        return response2Client(fd, 530);
    }
}

int handleQUIT(int fd) {
    // response with 221
	if (response2Client(fd, 221) == -1) return -1;
    else return -2;
}

int validCmd(char* cmd) {
    if (strcmp(cmd, "USER")
        && strcmp(cmd, "PASS")
        && strcmp(cmd, "QUIT")) return 0;
    else return 1;
}

int cmdMapper(int fd, char* cmd, char* param) {
	// mapping
	if (!strcmp(cmd, "USER")) return handleUSER(fd, param);
	else if (!strcmp(cmd, "PASS")) return handlePASS(fd, param);
	else if (!strcmp(cmd, "QUIT")) return handleQUIT(fd);
	else {
		// response with 500
		return response2Client(fd, 500);
	}
}