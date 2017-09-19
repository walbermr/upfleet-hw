#include <stdio.h>
#include "tcpclient.hpp"

int main(int argc , char *argv[])
{
    char *server_reply;

    SOCKET scoket;

    initWINSOCK();
    initSocket(&scoket);
    connect(scoket, "192.168.25.13", 5000); //ip do localhost

    while(true)
    {
    	sendData(scoket, "teste");
    	server_reply = recData(scoket, 1024, true);
    	if(server_reply == NULL)
    	{
    		printf("Servidor desconectado.\n");
    		return 0;
    	}
    	printf(server_reply);
    }

    return 0;
}