#include <stdio.h>
#include <string.h>
#include "./ipc/tcpclient.hpp"
#include "./sketch/abrasion.h"

#define	BUF_SIZE	4

unsigned char count;

void printHex(unsigned char *buf);
void decode(unsigned char *msg, short *rpm_engine_value, short *speed, short *brk);

int main(int argc , char *argv[])
{
	unsigned char *server_reply;
	short speed[BUF_SIZE], rpm_engine_value[BUF_SIZE], brk[BUF_SIZE];

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

			decode(server_reply, &rpm_engine_value[count % BUF_SIZE], &speed[count % BUF_SIZE], &brk[count % BUF_SIZE]);
			printf("engine: %d\n", rpm_engine_value[count % BUF_SIZE]);

			accumulateWear(rpm_engine_value, speed, brk, count % BUF_SIZE);
			count += 1;
		}

		sendData(scoket, "Done");
		resetWear(4);
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

	printHex(str);

	*rpm_engine_value = (msg[0] << 8) | msg[1];
	
	*speed = (msg[2] << 8) | msg[3];

	*brk = (msg[4] << 8) | msg[5];

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
