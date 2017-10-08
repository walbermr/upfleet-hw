#include <stdio.h>
#include <string.h>
#include "./ipc/tcpclient.hpp"
#include "./sketch/abrasion.h"


unsigned char count;

void printHex(unsigned char *buf);
void decode(unsigned char *msg, short *rpm_engine_value, short *speed, short *brk);

int main(int argc , char *argv[])
{
	unsigned char *server_reply;
	short speed, rpm_engine_value, brk;

	SOCKET scoket;

	initWINSOCK();
	initSocket(&scoket);
	connect(scoket, "192.168.25.13", 5000); //ip do localhost

	while(true)
	{
		count = 0;
		while(count < 4096)
		{
			server_reply = (unsigned char *) recData(scoket, 6, true);
			if(server_reply == NULL)
			{
				printf("Servidor desconectado.\n");
				return 0;
			}
			sendData(scoket, "ACK");
			//printHex(server_reply);
			/*
				DECODIFICA MENSAGEM
			*/
			decode(server_reply, &speed, &rpm_engine_value, &brk);
			printf("engine: %d\n", rpm_engine_value);

			/*
				CALCULA DESGASTE
			*/
			accumulateWear(rpm_engine_value, 0, 0);
			count += 1;
		}

		sendData(scoket, "Done");

		count = 0;
	}

	return 0;
}

void decode(unsigned char *msg, short *rpm_engine_value, short *speed, short *brk)
{
	int i = 0;
	unsigned char str[] = {msg[0], msg[1], msg[2], msg[3], msg[4], msg[5]};
	*rpm_engine_value = 0;
	*speed = 0;
	*brk = 0;

	for(int j = 1; i < 2; i++, j--)
		*rpm_engine_value += (msg[i] << (j*8));

	printHex(str);
	for(int j = 1; i < 4; i++, j--)
		*speed += (msg[i] << (j*8));

	for(int j = 1; i < 6; i++, j--)
		*brk += (msg[i] << (j*8));

	printf("\n");
	return;
}


void printHex(unsigned char *buf)
{
	int sum = 0;
	for (int i = 0; i < 6; i++)
	{
		if (i > 0) printf(":");
		printf("%02X", buf[i]);
		sum += buf[i];
	}

	printf("\n");
	return;
}
