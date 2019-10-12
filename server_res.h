#ifndef SERVER_RES_H
#define SERVER_RES_H

#include "server_util.h"

char response150[MAXRES];
char response200[MAXRES];
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
const char response451[] = "451 Fail to open/create file!\r\n";
const char response500[] = "500 Syntax error!\r\n";
const char response503[] = "503 \r\n";
const char response504[] = "504 Parameters not supported!\r\n";
const char response530[] = "530 Username is unacceptable (only support \"anonymous\" now)!\r\n";
const char response550[] = "550 Directory operation fail! Please check the parameter.\r\n";

#endif