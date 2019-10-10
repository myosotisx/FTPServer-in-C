#include "server_pi.h"
#include "server_util.h"

#include <string.h>

#include <stdio.h>

#define MAXRES 1024

char response150[MAXRES];
char response200[MAXRES]; //= "200 TYPE set to I.\r\n";
const char response220[] = "220 FTP server ready.\r\n";
const char response215[] = "215 UNIX Type: L8\r\n";
const char response221[] = "221-Thanks for using my FTP service.\r\n221 Goodbye.\r\n";
const char response226[] = "226 Transfer complete.\r\n";
char response227[MAXRES];
const char response230[] = "230-\r\n230-Welcome to stx's FTP server!\r\n230-You can download what ever you want here.\r\n230-\r\n230 Guest login ok, access restrictions apply.\r\n";
char response250[MAXRES];
char response257[MAXRES];
const char response331[] = "331 Please send your complete e-mail address as password.\r\n";
const char response332[] = "332 Please send your username to log in (only support \"anonymous\" now).\r\n";
const char response350[] = "350 RNFR accepted - file exists, ready for destination.\r\n";
const char response425[] = "425 No data connection established!\r\n";
const char response426[] = "426 Data connection is broken!\r\n";
const char response451[] = "451 Fail to open file!\r\n";
const char response500[] = "500 Syntax error!\r\n";
const char response504[] = "504 Parameters not supported!\r\n";
const char response530[] = "530 Username is unacceptable (only support \"anonymous\" now)!\r\n";
const char response550[] = "550 Directory operation fail! Please check the parameter.\r\n";

const char* getResponse(int code) {
    printf("Debug Info: response code is %d\r\n", code);
    switch (code) {
        case 150: return response150;
        case 200: return response200;
        case 220: return response220;
        case 215: return response215;
        case 221: return response221;
        case 226: return response226;
        case 227: return response227;
        case 230: return response230;
        case 250: return response250;
        case 257: return response257;
        case 331: return response331;
        case 332: return response332;
        case 350: return response350;
        case 425: return response425;
        case 426: return response426;
        case 451: return response451;
        case 500: return response500;
        case 504: return response504;
        case 530: return response530;
        case 550: return response550;
        default: return response500;
    }
}

int handleUSER(int fd, char* param) {
    setUsername(fd, param);
	// response with 331
	return response(fd, 331);
}

int handlePASS(int fd, char* param) {
    // check username
    if (!strcmp(getUsername(fd), "anonymous")) {
        setPassword(fd, param);
        // response with 230
	    return response(fd, 230);
    }
    else {
        // response with 530
        return response(fd, 530);
    }
}

int handleQUIT(int fd) {
    // response with 221
	if (response(fd, 221) == -1) return -1;
    else return -2;
}

int handleABOR(int fd) {
    // response with 221 same as QUIT
    if (response(fd, 221) == -1) return -1;
    else return -2;
}

int handleSYST(int fd) {
    // response with 215
    return response(fd, 215);
}

int handleTYPE(int fd, char* param) {
    if (!strcmp(param, "I")) {
        sprintf(response200, "200 TYPE set to I.\r\n");
        // response with 200
        return response(fd, 200);
    }
    // response with 504
    else return response(fd, 504);
}

int handlePORT(int fd, char* param) {
    char ipAddr[32];
    int port;
    parseIpAddrNPort(param, ipAddr, &port);
    // response with 425
    if (enterPortMode(fd, ipAddr, port) == -1) return response(fd, 425);
    sprintf(response200, "200 PORT command sccessful.\r\n");
    // response with 200
    if (response(fd, 200) == -1) return -1;
    else return 1;
}

int handlePASV(int fd) {
    char ipAddr[32];
    int port;
    memset(ipAddr, 0, 32);
    if (enterPassiveMode(fd, ipAddr, &port) != -1) {
        short p1, p2;
        p1 = port/256;
        p2 = port%256;
        strReplace(ipAddr, '.', ',');
        sprintf(response227, "227 Entering Passive Mode (%s,%d,%d)\r\n", ipAddr, p1, p2);
        // response with 227
        return response(fd, 227);
    }
    // response with 425
    else return response(fd, 425);
}

int handleRETR(int fd, char* param) {
    char path[MAXPATH];
    getFilePath(fd, path, param);
    
    FILE* file = fopen(path, "rb");
    if (file) {
        sprintf(response150, "150 Opening BINARY mode data connection for %s (%u bytes).\r\n", param, getFileSize(file));
        // response with 150
        if (response(fd, 150) == -1) {
            fclose(file);
            return -1;
        }

        if (getDataMode(fd) == 0) {
            // PORT mode
            if (setupDataConn(fd) == -1) {
                fclose(file);
                // response with 425
                return response(fd, 425);
            }
            setClientTransfer(fd, 1);
            if (writeFile(fd, file) == -1) {
                closeDataConn(fd);
                setClientTransfer(fd, 0);
                fclose(file);
                // response with 426
                return response(fd, 426);
            }
            closeDataConn(fd);
            setClientTransfer(fd, 0);
            fclose(file);
            return response(fd, 226);
        }
        else if (getDataMode(fd) == 1) {
            // PASV mode
            if (getDataConnfd(fd) == -1 || getDataListenfd(fd) == -1) {
                fclose(file);
                // response with 425
                return response(fd, 425);
            }
            setClientTransfer(fd, 1);
            if (writeFile(fd, file) == -1) {
                closeDataConn(fd);
                setClientTransfer(fd, 0);
                fclose(file);
                // response with 426
                return response(fd, 426);
            }
            closeDataConn(fd);
            setClientTransfer(fd, 0);
            fclose(file); 
            return response(fd, 226);
        }
        // response with 425
        else return response(fd, 425);
    }
    else {
        // response with 451
        return response(fd, 451);
    }
    
}

int handlePWD(int fd) {
    char formatPath[MAXPATH];
    getFormatPath(formatPath, getWorkDir(fd));
    sprintf(response257, "257 \"%s\" is your current location.\r\n", formatPath);
    // response with 257
    return response(fd, 257);
}

int handleMKD(int fd, char* param) {
    if (makeDir(fd, param) != -1) {
        char formatPath[MAXPATH];
        getFormatPath(formatPath, param);
        sprintf(response257, "257 \"%s\" : The directory was successfully created.\r\n", formatPath);
        // response with 257
        return response(fd, 257);
    }
    else {
        // response with 550
        return response(fd, 550);
    }
}

int handleCWD(int fd, char* param) {
    if (changeWorkDir(fd, param) != -1) {
        char formatPath[MAXPATH];
        getFormatPath(formatPath, getWorkDir(fd));
        sprintf(response250, "250 OK. Current directory is %s\r\n", formatPath);
        // response with 250
        return response(fd, 250);
    }
    else {
        // response with 550
        return response(fd, 550);
    }
}

int handleRMD(int fd, char* param) {
    if (removeDir(fd, param) != -1) {
        sprintf(response250, "250 The directory was successfully removed.\r\n");
        // response with 250
        return response(fd, 250);
    }
    else {
        // response with 550
        return response(fd, 550);
    }
}

int handleRNFR(int fd, char* param) {
    if (setFile2Rename(fd, param) != -1) {
        // response with 350
        return response(fd, 350);
    }
    else {
        // response with 550
        return response(fd, 550);
    }
}

int handleRNTO(int fd, char* param) {
    if (renameFile(fd, "", param) != -1) {
        sprintf(response250, "250 File successfully renamed or moved.\r\n");
        // response with 250
        return response(fd, 250);
    }
    else {
        // response with 550
        return response(fd, 550);
    }
}

int validCmd(char* cmd) {
    if (strcmp(cmd, "USER")
        && strcmp(cmd, "PASS")
        && strcmp(cmd, "QUIT")) return 0;
    else return 1;
}

int cmdMapper(int fd, char* cmd, char* param) {
    printf("Debug Info in PI: cmd is %s and param is %s\r\n", cmd, param);
	// mapping
	if (!strcmp(cmd, "USER")) return handleUSER(fd, param);
	else if (!strcmp(cmd, "PASS")) return handlePASS(fd, param);
	else if (!strcmp(cmd, "QUIT")) return handleQUIT(fd);
    else if (!strcmp(cmd, "ABOR")) return handleABOR(fd);
    else if (!strcmp(cmd, "SYST")) return handleSYST(fd);
    else if (!strcmp(cmd, "TYPE")) return handleTYPE(fd, param);
    else if (!strcmp(cmd, "PORT")) return handlePORT(fd, param);
    else if (!strcmp(cmd, "PASV")) return handlePASV(fd);
    else if (!strcmp(cmd, "RETR")) return handleRETR(fd, param);
    else if (!strcmp(cmd, "PWD")) return handlePWD(fd);
    else if (!strcmp(cmd, "MKD")) return handleMKD(fd, param);
    else if (!strcmp(cmd, "CWD")) return handleCWD(fd, param);
    else if (!strcmp(cmd, "RMD")) return handleRMD(fd, param);
    else if (!strcmp(cmd, "RNFR")) return handleRNFR(fd, param);
    else if (!strcmp(cmd, "RNTO")) return handleRNTO(fd, param);
	else {
		// response with 500
		return response(fd, 500);
	}
}