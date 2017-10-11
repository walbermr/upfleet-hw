#include "abrasion.h"

int BRAKEWEAR[] = {1000, 2000, 3000};
int CLUTCHWEAR[] = {500, 1500, 2000};
int ENGINEWEAR[] = {2000, 3000, 4500};

unsigned char BRAKE_WEAR[36]	= {};
unsigned char CLUTCH_WEAR[8]	= {};
unsigned char ENGINE_WEAR[16]	= {};

unsigned char CUMULATIVE_BRAKEWEAR[] = {0, 0, 0, 0};
unsigned char CUMULATIVE_CLUTCHWEAR[] = {0, 0, 0, 0};
unsigned char CUMULATIVE_ENGINEWEAR[] = {0, 0, 0, 0};


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


void accumulateWear(short rpm_sample[], short speed_sample[], short brake_sample[], short i) {	//acumula valores de desgaste
	char brk_time, spd, brk_rate, rpm_rate, is_brk, rpm_time, rpm;

	brk_time = discretize(brake_sample[i], BRAKEWEAR, 3);
	spd = discretize(speed_sample[i], CLUTCHWEAR, 3);
	brk_rate = rate(brake_sample[i], brake_sample[(BUF_SIZE + i) % BUF_SIZE], BRAKEWEAR);

	rpm_rate = rate(rpm_sample[i], rpm_sample[(BUF_SIZE + i) % BUF_SIZE], ENGINEWEAR);
	is_brk = brake_sample[i] > 0;

	rpm_time = discretize(rpm_sample[i], ENGINEWEAR, 3);
	rpm = discretize(rpm_sample[i], ENGINEWEAR, 3);

	CUMULATIVE_BRAKEWEAR[verifyWear({brk_time, spd, brk_rate}, {2, 2, 2}, 3, BRAKE_WEAR)] += 1;
	CUMULATIVE_CLUTCHWEAR[verifyWear({rpm_rate, is_brk}, {2, 1}, 2, CLUTCH_WEAR)] += 1;
	CUMULATIVE_ENGINEWEAR[verifyWear({rpm_time, rpm}, {2, 2}, 2, ENGINE_WEAR)] += 1;

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


char rate(char x1, char x2, int vect[]) {
	char dx = discretize(x2, vect, 3) - discretize(x1, vect, 3);

	return (dx >= 0)? dx: -dx;
}
