#include <stdio.h>
#include <string.h>
#include <winsock2.h>
#include <windows.h>
#include "./ipc/tcpclient.hpp"
#include "./sketch/abrasion.h"

const char IP[] = "192.168.25.5";	//MODIFIQUE O IP ANTES DE EXECUTAR

unsigned short count;

void printHex(unsigned char *buf, char size);
void decode(unsigned char *msg, short *rpm_engine_value, short *speed, short *brk);

int main(int argc , char *argv[])
{
	unsigned char *server_reply, data[2];
	short speed, rpm_engine_value, brk;
	char ack[] = "ok";
	FILE *wear = fopen("wear.txt", "w");
	short sample = 1024;

	fprintf(wear, "wear = {sample_size: %d, values = [\n", sample);

	/* Inicialização do socket TCP */
	SOCKET scoket;
	initWINSOCK();
	initSocket(&scoket);
	connect(scoket, IP, 5000); //ip do localhost
	//

	while(true)
	{
		count = 0;
		while(count < sample)
		{
			server_reply = (unsigned char *) recData(scoket, 6, true);
			if(server_reply == NULL)
			{
				printf("Servidor desconectado.\n");
				fprintf(wear, "]}");
				return 0;
			}
			sendData(scoket, ack);

			decode(server_reply, &rpm_engine_value, &speed, &brk);

			accumulateWear(rpm_engine_value, speed, brk);
			count += 1;
		}

		wearData(data);				//calcula o desgaste e guarda na variavel data
		fprintf(wear, "{brake: %u, clutch: %u, engine: %u},\n", data[0]>>4, (data[0]>>2) & 0x3, data[0] & 0x3);
		data[0] = data[0] | 0xC0;	//envia pelo menos 2 bits com 1 por conta do tcp
		sendData(scoket, (char*) data);
		
		printf("Data sent: ");
		printHex(data, 1);
		Sleep(100);				//delay pra ver o q ta acontecendo

		resetWear(4);
	}

	return 0;
}


void decode(unsigned char *msg, short *rpm_engine_value, short *speed, short *brk)	//decodifica os dados enviados do servidor
{
	int i = 0;
	unsigned char str[] = {msg[0], msg[1], msg[2], msg[3], msg[4], msg[5]};
	*rpm_engine_value = 0;
	*speed = 0;
	*brk = 0;

	printHex(str, 6);

	*rpm_engine_value = (msg[0] << 8) | msg[1];
	
	*speed = (msg[2] << 8) | msg[3];

	*brk = (msg[4] << 8) | msg[5];

	printf("\n");
	return;
}


void printHex(unsigned char *buf, char size)
{
	int sum = 0;
	for (int i = 0; i < size; i++)
	{
		if (i > 0) printf(":");
		printf("%02X", buf[i]);
		sum += buf[i];
	}

	printf("\n");
	return;
}
