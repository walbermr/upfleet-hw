#include "abrasion.h"

int ENGINEWEAR[] = {2000, 3000, 4500};
int CLUTCHWEAR[] = {500, 1500, 2000};
int BRAKEWEAR[] = {1000, 2000, 3000};

unsigned char CUMULATIVE_ENGINEWEAR[] = {0, 0, 0, 0};
unsigned char CUMULATIVE_CLUTCHWEAR[] = {0, 0, 0, 0};
unsigned char CUMULATIVE_BREAKWEAR[] = {0, 0, 0, 0};


char discretize(int value, int thresh[], char len) {
	char out;

	for (out = 0; out < len; out++) {
		if (value <= thresh[out]) {
			break;
		}
	}

	return out;
}


char verifyWear(char param[], char param_bits[], char n_param, char wear[]) {
	char i, in = 0;

	for (i = 0; i < n_param; i++) {
		in <<= param_bits[i];
		in += param[i];
	}

	return wear[in];
}


void accumulateWear(int engine, int clutch, int brake) {	//acumula valores de desgaste
	CUMULATIVE_ENGINEWEAR[discretize(engine, ENGINEWEAR, 3)] += 1;
	CUMULATIVE_CLUTCHWEAR[discretize(clutch, CLUTCHWEAR, 3)] += 1;
	CUMULATIVE_BREAKWEAR[discretize(brake, BRAKEWEAR, 3)] += 1;

	return;
}


void resetWear(char v_len) {
	unsigned char *v[] = {CUMULATIVE_ENGINEWEAR, CUMULATIVE_CLUTCHWEAR, CUMULATIVE_BREAKWEAR};
	for(char i = 0; i < 3; i++)
	{
		for(char j = 0; j < v_len; j++)
			(v+i)[j] = 0;
	}

	return;
}
