#include <stdio.h>
#include "tcpclient.hpp"

void printHex(unsigned char *buf)
{
    int sum = 0;
    for (int i = 0; i < 4; i++)
    {
        if (i > 0) printf(":");
        printf("%02X", buf[i]);
        sum += buf[i];
    }

    printf("\n");
    return;
}

int main(int argc , char *argv[])
{
    unsigned char *server_reply;

    SOCKET scoket;

    initWINSOCK();
    initSocket(&scoket);
    connect(scoket, "192.168.25.13", 5000); //ip do localhost

    while(true)
    {
        server_reply = (unsigned char *) recData(scoket, 4, true);
        if(server_reply == NULL)
        {
            printf("Servidor desconectado.\n");
            return 0;
        }

        printHex(server_reply);
    	sendData(scoket, "teste");
    }

    return 0;
}