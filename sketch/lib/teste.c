#include <stdio.h>
#include <string.h>
#include <stdlib.h>

union Pos {  // union consegue definir vários tipos de dados na mesma posição de memória
	float f;
	char b[4];
};

void printHex(unsigned char *buf, char size);

int main()
{
	char msg[12];
	char desgaste = 5;
	union Pos lon, lat;

	lat.f = -8.056157;
	lon.f = -34.951114;

	memcpy(msg, &desgaste, 1);
	memcpy(msg+1, lat.b, 4);
	memcpy(msg+5, lon.b, 4);
	//strcat(msg, lon.b);

	printHex((unsigned char*) lat.b, 4);
	printHex((unsigned char*) lon.b, 4);
	printHex((unsigned char*) msg, 12);
	return 0;
}

void printHex(unsigned char *buf, char size)
{
	int sum = 0, i;
	for (i = 0; i < size; i++)
	{
		if (i > 0) printf(":");
		printf("%02X", buf[i]);
		sum += buf[i];
	}

	printf("\n");
	return;
}