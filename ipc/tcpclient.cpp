/*
    Create a TCP socket
*/
#include "tcpclient.hpp"

int initWINSOCK()
{
    WSADATA wsa;

    printf("\nInitialising Winsock...");
    if (WSAStartup(MAKEWORD(2,2),&wsa) != 0)
    {
        printf("Failed. Error Code : %d",WSAGetLastError());
        return 1;
    }
     
    printf("Initialised.\n");

    return 0;
}

void initSocket(SOCKET *s)
{
    //Create a socket
    if((*s = socket(AF_INET , SOCK_STREAM , 0 )) == INVALID_SOCKET)
    {
        printf("Could not create socket : %d" , WSAGetLastError());
    }
 
    printf("Socket created.\n");
    return;
}


int connect(SOCKET s, char const *ip, int port)
{
    struct sockaddr_in server;

    server.sin_addr.s_addr = inet_addr(ip);
    server.sin_family = AF_INET;
    server.sin_port = htons( port );
 
    //Connect to remote server
    if (connect(s , (struct sockaddr *)&server , sizeof(server)) < 0)
    {
        printf("connect error");
        return 1;
    }
     
    printf("Connected");
    return 0;
}

int sendData(SOCKET s, char const *message)
{
    if( send(s , message , strlen(message) , 0) < 0)
    {
        printf("Send failed");
        return 1;
    }
    printf("Data Send\n");
    return 0;
}

char *recData(SOCKET s, int size, bool too_string)
{
    int recv_size;
    char *server_reply;
    if(too_string)
    {
        server_reply = (char *) malloc((size+1)*sizeof(char));
    }
    else
    {
        server_reply = (char *) malloc(size*sizeof(char));
    }

    if((recv_size = recv(s , server_reply , size , 0)) == SOCKET_ERROR)
    {
        printf("Recv failed\n");
        return NULL;
    }
         
    printf("Reply received\n");

    if(too_string)
    {
        server_reply[recv_size] = '\0';
    }

    return server_reply;
}