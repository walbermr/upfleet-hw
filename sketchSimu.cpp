#include <stdio.h>
#include "./ipc/tcpclient.hpp"
#include "./sketch/abrasion.h"

unsigned char count = 0;

void printHex(unsigned char *buf);
void decode(unsigned char *msg, int *speed, int *rpm_engine_value, int *brk);

int main(int argc , char *argv[])
{
	unsigned char *server_reply;
	int speed, rpm_engine_value, brk;

	SOCKET scoket;

	initWINSOCK();
	initSocket(&scoket);
	connect(scoket, "192.168.25.13", 5000); //ip do localhost

	while(true)
	{
		while(count < 4096)
		{
			server_reply = (unsigned char *) recData(scoket, 4, true);
			if(server_reply == NULL)
			{
				printf("Servidor desconectado.\n");
				return 0;
			}
		}
		//printHex(server_reply);
		/*
			DECODIFICA MENSAGEM
		*/
		decode(server_reply, &speed, &rpm_engine_value, &brk);

		/*
			CALCULA DESGASTE
		*/
		accumulateWear(rpm_engine_value, 0, 0);

		sendData(scoket, "teste");
	}

	return 0;
}

void decode(unsigned char *msg, int *rpm_engine_value, int *speed, int *brk)
{
	*rpm_engine_value = msg[3];
	*speed = msg[2];
	*brk = msg[0];
	*brk += msg[1]<<4;

	printf("%d\n", rpm_engine_value);

	return;
}

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
