#ifndef TCPCLIENT
#define TCPCLIENT

#include <winsock2.h>
#include <stdio.h>

int initWINSOCK();
void initSocket(SOCKET *s);
int connect(SOCKET s, char const *ip, int port);
int sendData(SOCKET s, char const *message);
char *recData(SOCKET s, int size, bool too_string);

#endif