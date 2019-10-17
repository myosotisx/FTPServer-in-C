#ifndef SERVER_RES_H
#define SERVER_RES_H

#include "server_util.h"

char response150[MAXRES];
char response200[MAXRES];
char response215[MAXRES] = "215 UNIX Type: L8\r\n";
char response220[MAXRES] = "220 FTP server ready.\r\n";
char response221[MAXRES] = "221-Thanks for using my FTP service.\r\n221 Goodbye.\r\n";
char response226[MAXRES] = "226 Transfer complete.\r\n";
char response227[MAXRES];
char response230[MAXRES] = "230-\r\n230-Welcome to stx's FTP server!\r\n230-You can download what ever available here.\r\n230-\r\n230 Guest login ok, access restrictions apply.\r\n";
char response250[MAXRES];
char response257[MAXRES];
char response331[MAXRES] = "331 Please send your complete e-mail address as password.\r\n";
char response332[MAXRES] = "332 Please send your username to log in (only support \"anonymous\" now).\r\n";
char response350[MAXRES];
char response425[MAXRES] = "425 No data connection established!\r\n";
char response426[MAXRES] = "426 Data connection is broken!\r\n";
char response451[MAXRES] = "451 Fail to open/create file!\r\n";
char response500[MAXRES] = "500 Syntax error!\r\n";
char response502[MAXRES] = "502 Fail to execute command!\r\n";
char response503[MAXRES] = "503 Bad sequence of commands.\r\n";
char response504[MAXRES] = "504 Parameters not supported!\r\n";
char response530[MAXRES] = "530 Username is unacceptable (only support \"anonymous\" now)!\r\n";
char response550[MAXRES];

#endif