#include "server_pi.h"
#include "server_util.h"
#include "server_res.h"

#include <unistd.h>
#include <string.h>
#include <pthread.h>

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
        case 503: return response503;
        case 504: return response504;
        case 530: return response530;
        case 550: return response550;
        default: return response500;
    }
}

void sendFile(void* _fd) {
	int fd = *(int*)_fd;
	FILE* file = getReservedPtr(fd, 0);
    int mode = getDataMode(fd);
    int conn = 0;
    int res;

    if (mode == 0) {
        // PORT mode
        // 等待客户端开启监听，timeout为1s
        sleep(1);
        if (setupDataConn(fd, 1) == -1) {
            // response with 425
            res = response(fd, 425);
        }
        else conn = 1;
    }
    else if (mode == 1) {
        // PASV mode
        // 等待客户端建立连接，timeout为1s
        sleep(1);
        if (getDataConnfd(fd) == -1 || getDataListenfd(fd) == -1) {
            // response with 425
            res = response(fd, 425);
        }
        else conn = 1;
    }
    // response with 425
    else res = response(fd, 425);

    if (conn) {
        unsigned char fileBuf[MAXBUF];
        int readLen, writeLen;
        int dataConnfd = getDataConnfd(fd);
        while ((readLen = fread(fileBuf, sizeof(unsigned char), MAXBUF, file))) {
            if ((writeLen = writeBuf(dataConnfd, fileBuf, readLen)) == -1) {
                break;
            }
        }
        closeDataConn(fd);
        pthread_testcancel();
        // response with 426
        if (writeLen == -1) res = response(fd, 426);
        // response with 226
        else res = response(fd, 226);
    }
    fclose(file);
    setTransThread(fd, 0);
	setClientState(fd, res);
}

void recvFile(void* _fd) {
    int fd = *(int*)_fd;
	FILE* file = getReservedPtr(fd, 1);
    int mode = getDataMode(fd);
    int conn = 0;
    int res;

    if (mode == 0) {
        // PORT mode
        // 等待客户端开启监听，timeout为1s
        sleep(1);
        if (setupDataConn(fd, 1) == -1) {
            // response with 425
            res = response(fd, 425);
        }
        else conn = 1;
    }
    else if (mode == 1) {
        // PASV mode
        // 等待客户端建立连接，timeout为1s
        sleep(1);
        if (getDataConnfd(fd) == -1 || getDataListenfd(fd) == -1) {
            // response with 425
            res = response(fd, 425);
        }
        else conn = 1;
    }
    // response with 425
    else res = response(fd, 425);

    if (conn) {
        unsigned char fileBuf[MAXBUF];
        int readLen;
        int dataConnfd = getDataConnfd(fd);
        while ((readLen = read(dataConnfd, fileBuf, MAXBUF))) {
            if (readLen == -1) break;
            fwrite(fileBuf, sizeof(unsigned char), readLen, file);
        }
        closeDataConn(fd);
        pthread_testcancel();
        // response with 426
        if (readLen == -1) res = response(fd, 426);
        // response with 226
        else res = response(fd, 226);
    }
    fclose(file);
    setTransThread(fd, 0);
	setClientState(fd, res);
}

void sendFileList(void* _fd) {
    int fd = *(int*)_fd;
    const char* param = getReserved(fd, 1);
    int mode = getDataMode(fd);
    int conn = 0;
    int res;

    if (mode == 0) {
        // PORT mode
        // 等待客户端开启监听，timeout为1s
        sleep(1);
        if (setupDataConn(fd, 1) == -1) {
            // response with 425
            res = response(fd, 425);
        }
        else conn = 1;
    }
    else if (mode == 1) {
        // PASV mode
        // 等待客户端建立连接，timeout为1s
        sleep(1);
        if (getDataConnfd(fd) == -1 || getDataListenfd(fd) == -1) {
            // response with 425
            res = response(fd, 425);
        }
        else conn = 1;
    }
    // response with 425
    else res = response(fd, 425);

    if (conn) {
        char fileList[MAXBUF];
        getFileList(fd, fileList, param);
        int dataConnfd = getDataConnfd(fd);
        int writeLen = writeBuf(dataConnfd, fileList, strlen(fileList));
        closeDataConn(fd);
        // response with 426
        pthread_testcancel();
        if (writeLen == -1) res = response(fd, 426);
        // response with 226
        else res = response(fd, 226);
    }
    setTransThread(fd, 0);
    setClientState(fd, res);
}

int handleUSER(int fd, char* param) {
    setUsername(fd, param);
	// response with 331
	if (response(fd, 331) != -1) return 4;
    else return -1;
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
        if (response(fd, 530) != -1) return 3;
        else return -1;
    }
}

int handleQUIT(int fd) {
    // response with 221
	if (response(fd, 221) == -1) return -1;
    else return 0;
}

int handleABOR(int fd) {
    // response with 221 same as QUIT
    if (response(fd, 221) == -1) return -1;
    else return 0;
}

int handleSYST(int fd) {
    // response with 215
    return response(fd, 215);
}

int handleTYPE(int fd, char* param) {
    if (!strcmp(param, "I")) {
        memset(response200, 0, MAXRES);
        sprintf(response200, "200 Type set to I.\r\n");
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
    memset(response200, 0, MAXRES);
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
        memset(response227, 0, MAXRES);
        sprintf(response227, "227 Entering Passive Mode (%s,%d,%d)\r\n", ipAddr, p1, p2);
        // response with 227
        return response(fd, 227);
    }
    // response with 425
    else return response(fd, 425);
}

int handleRETR(int fd, char* param) {
    char filePath[MAXPATH];
    getFilePath(fd, filePath, param);
    
    FILE* file = fopen(filePath, "rb");
    if (file) {
        memset(response150, 0, MAXRES);
        sprintf(response150, "150 Opening BINARY mode data connection for %s (%lld bytes).\r\n", param, getFileSize(file));
        // response with 150
        if (response(fd, 150) == -1) {
            fclose(file);
            return -1;
        }
        setReservedPtr(fd, 0, file);
        pthread_t transThread;
        if (pthread_create(&transThread, NULL, (void*)sendFile, &fd) != -1
            && setTransThread(fd, transThread) != -1) {
            return 2;
        }
        // response with 425
        else return response(fd, 425);
    }
    else {
        // response with 451
        return response(fd, 451);
    }
    
}

int handleSTOR(int fd, char* param) {
    char filePath[MAXPATH];
    getFilePath(fd, filePath, param);

    FILE* file = fopen(filePath, "wb");
    if (file) {
        memset(response150, 0, MAXRES);
        sprintf(response150, "150 Opening BINARY mode data connection.\r\n");
        // response with 150
        if (response(fd, 150) == -1) {
            fclose(file);
            return -1;
        }
        setReservedPtr(fd, 1, file);
        pthread_t transThread;
        if (pthread_create(&transThread, NULL, (void*)recvFile, &fd) != -1
            && setTransThread(fd, transThread) != -1) {
            return 2;
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
    memset(response257, 0, MAXRES);
    sprintf(response257, "257 \"%s\" is your current location.\r\n", formatPath);
    // response with 257
    return response(fd, 257);
}

int handleMKD(int fd, char* param) {
    if (makeDir(fd, param) != -1) {
        char formatPath[MAXPATH];
        getFormatPath(formatPath, param);
        memset(response257, 0, MAXRES);
        sprintf(response257, "257 \"%s\" : The directory was successfully created.\r\n", formatPath);
        // response with 257
        return response(fd, 257);
    }
    else {
        memset(response550, 0, MAXRES);
        sprintf(response550, "550 Fail to create directory!\r\n");
        // response with 550  
        return response(fd, 550);
    }
}

int handleCWD(int fd, char* param) {
    if (changeWorkDir(fd, param) != -1) {
        char formatPath[MAXPATH];
        getFormatPath(formatPath, getWorkDir(fd));
        memset(response250, 0, MAXRES);
        sprintf(response250, "250 OK. Current directory is \"%s\"\r\n", formatPath);
        // response with 250
        return response(fd, 250);
    }
    else {
        memset(response550, 0, MAXRES);
        sprintf(response550, "550 Fail to change directory!\r\n");
        // response with 550
        return response(fd, 550);
    }
}

int handleRMD(int fd, char* param) {
    if (removeDir(fd, param) != -1) {
        memset(response250, 0, MAXRES);
        sprintf(response250, "250 The directory was successfully removed.\r\n");
        // response with 250
        return response(fd, 250);
    }
    else {
        memset(response550, 0, MAXRES);
        sprintf(response550, "550 Fail to remove directory!\r\n");
        // response with 550
        return response(fd, 550);
    }
}

int handleRNFR(int fd, char* param) {
    if (setFile2Rename(fd, param) != -1) {
        // response with 350
        if (response(fd, 350) != -1) return 5;
        else return -1;
    }
    else {
        memset(response550, 0, MAXRES);
        sprintf(response550, "550 Sorry, but file/directory doesn't exist.\r\n");
        // response with 550
        return response(fd, 550);
    }
}

int handleRNTO(int fd, char* param) {
    if (renameFile(fd, "", param) != -1) {
        memset(response250, 0, MAXRES);
        sprintf(response250, "250 Successfully renamed or moved file/directory.\r\n");
        // response with 250
        return response(fd, 250);
    }
    else {
        memset(response550, 0, MAXRES);
        sprintf(response550, "550 Fail to rename file/directory.\r\n");
        // response with 550
        return response(fd, 550);
    }
}

int handleLIST(int fd, char* param) {
    memset(response150, 0, MAXRES);
    sprintf(response150, "150 Connection setup.\r\n");
    // response with 150
    if (response(fd, 150) == -1) return -1;
    setReserved(fd, 1, param);
    pthread_t transThread;
    if (pthread_create(&transThread, NULL, (void*)sendFileList, &fd) != -1
        && setTransThread(fd, transThread) != -1) {
        return 2;
    }
    // response with 425
    else return response(fd, 425);
}

int validCmd(char* cmd) {
    if (!strcmp(cmd, "QUIT")
    || !strcmp(cmd, "ABOR")
    || !strcmp(cmd, "USER")
    || !strcmp(cmd, "PASS")
    || !strcmp(cmd, "RNTO")
    || !strcmp(cmd, "SYST")
    || !strcmp(cmd, "TYPE")
    || !strcmp(cmd, "PORT")
    || !strcmp(cmd, "PASV")
    || !strcmp(cmd, "RETR")
    || !strcmp(cmd, "STOR")
    || !strcmp(cmd, "PWD")
    || !strcmp(cmd, "MKD")
    || !strcmp(cmd, "CWD")
    || !strcmp(cmd, "RMD")
    || !strcmp(cmd, "RNFR")
    || !strcmp(cmd, "LIST")) return 1;
    else return 0;
}

int cmdMapper(int fd, char* cmd, char* param) {
    printf("Debug Info in PI: cmd is %s and param is %s\r\n", cmd, param);
	// mapping
    int res = getClientState(fd);
    if (!validCmd(cmd)) {
        int oRes = res;
        // response with 500
        if (oRes != 2) res = response(fd, 500);
        if (res != -1) res = oRes;
    }
    else if (!strcmp(cmd, "QUIT")) res = handleQUIT(fd);
    else if (!strcmp(cmd, "ABOR")) res = handleABOR(fd);
    else if (res == 3) {
        // 请求用户名
        if (!strcmp(cmd, "USER")) res = handleUSER(fd, param);
        // response with 332
        else response(fd, 332);
    }
    else if (res == 4) {
        // 请求密码
        if (!strcmp(cmd, "PASS")) res = handlePASS(fd, param);
        // response with 331
        else response(fd, 331);
    }
    else if (res == 2) {
        // 用户传输文件时不处理控制指令
        printf("Can not handle cmd when transfer!\r\n");
        return res;
    }
    else if (res == 5) {
        if (!strcmp(cmd, "RNTO")) res = handleRNTO(fd, param);
        // response with 503
        else res = response(fd, 503);
    }
    else if (!strcmp(cmd, "SYST")) res = handleSYST(fd);
    else if (!strcmp(cmd, "TYPE")) res = handleTYPE(fd, param);
    else if (!strcmp(cmd, "PORT")) res = handlePORT(fd, param);
    else if (!strcmp(cmd, "PASV")) res = handlePASV(fd);
    else if (!strcmp(cmd, "RETR")) res = handleRETR(fd, param);
    else if (!strcmp(cmd, "STOR")) res = handleSTOR(fd, param);
    else if (!strcmp(cmd, "PWD")) res = handlePWD(fd);
    else if (!strcmp(cmd, "MKD")) res = handleMKD(fd, param);
    else if (!strcmp(cmd, "CWD")) res = handleCWD(fd, param);
    else if (!strcmp(cmd, "RMD")) res = handleRMD(fd, param);
    else if (!strcmp(cmd, "RNFR")) res = handleRNFR(fd, param);
    else if (!strcmp(cmd, "LIST")) res = handleLIST(fd, param);
	/*else {
		// response with 500
		res = response(fd, 500);
	}*/
    setClientState(fd, res);
    return res;
}