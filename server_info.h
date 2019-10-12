#ifndef SERVER_INFO_H
#define SERVER_INFO_H

const char promptClientListen[] = "Prompt: server starts listening at port: %d.\r\n";
const char promptDataListen[] = "Prompt: server starts data listening at port: %d for client (fd: %d, dataListenfd: %d).\r\n";
const char promptDataConnReady[] = "Prompt: server data connection with client (fd: %d, ipAddr: %s, port: %d) is ready.\r\n";
const char promptTimeout[] = "Prompt: select Timeout.\r\n";
const char promptClientClose[] = "Prompt: connection with client (fd: %d) closed normally.\r\n";
const char promptReceive[] = "Prompt: receive from client (fd: %d):\r\n";
const char promptNewConn[] = "Prompt: new connection from client (fd: %d).\r\n";
const char promptNewDataConn[] = "Prompt: new data connection from client (fd: %d, dataListenfd: %d, dataConnfd: %d).\r\n";

const char errorListenFail[] = "Error: fail to setup listen at port: %d!\r\n";
const char errorSelect[] = "Error: error occurs when using select()! Server aborts!\r\n";
const char errorClientConn[] = "Error: connection with client fails!\r\n";
const char errorClientDataConn[] = "Error: data Connection with client (fd: %d, dataListenfd: %d) fails!\r\n";
const char errorInvalidReq[] = "Error: invalid Request!\r\n";
const char errorConnShutDown[] = "Error: connection with client (fd: %d) has been shut down!\r\n";
const char errorClientNotFound[] = "Error: client (fd: %d) not found!\r\n";

#endif