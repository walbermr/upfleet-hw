#include "abrasion.h"

int RPM_THRESHOLD[] = {500, 1500, 2000};
int SPD_THRESHOLD[] = {2000, 3000, 4500};
int BRK_THRESHOLD[] = {1000, 2000, 3000};

char BRAKE_WEAR[36]	= {};
char CLUTCH_WEAR[8]	= {};
char ENGINE_WEAR[16]	= {};

short CUMULATIVE_BRAKE[] = {0, 0, 0, 0};
short CUMULATIVE_CLUTCH[] = {0, 0, 0, 0};
short CUMULATIVE_RPM[] = {0, 0, 0, 0};

char last_brk, last_rpm;


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


void accumulateWear(short rpm, short spd, short brk) {	//acumula valores de desgaste
	char speed, brake_rate, rpm_rate, has_brake;

	speed = discretize(spd, SPD_THRESHOLD, 3);
	brake_rate = rate(last_brk, brk, BRK_THRESHOLD);

	rpm_rate = rate(last_rpm, rpm, RPM_THRESHOLD);
	has_brake = brk > 0;

	CUMULATIVE_BRAKE[verifyWear({speed, brake_rate}, {2, 2}, 2, BRAKE_WEAR)] += 1;
	CUMULATIVE_CLUTCH[verifyWear({rpm_rate, has_brake}, {2, 1}, 2, CLUTCH_WEAR)] += 1;
	CUMULATIVE_RPM[discretize(rpm, RPM_THRESHOLD, 3)] += 1;

	return;
}


void resetWear(char v_len) {
	unsigned char *v[] = {CUMULATIVE_RPM, CUMULATIVE_CLUTCH, CUMULATIVE_BRAKE};
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


char average(short buf[], int size) {
	int i;

	for (i = 0; i < 4; i++) {

	}
}


char* wearData() {
	char brake_wear, clutch_wear, engine_wear, rpm, rpm_time;

	rpm = average(CUMULATIVE_RPM);
	rpm_time = percent(CUMULATIVE_RPM, rpm, 4);

	brake_wear = average(CUMULATIVE_BRAKE);
	clutch_wear = average(CUMULATIVE_CLUTCH);
	engine_wear = verifyWear({rpm, rpm_time}, {2, 2}, 2, ENGINE_WEAR);

	return {brake_wear << 4 + clutch_wear << 2 + engine_wear ,'\0'};
}


char average(short vect[]) {
	int i, total = 0, value = 0, step;

	for (i = 0; i < 4; i++) {
		value += vect[i] * i;
		total += vect[i];
	}

	step = total / 2;

	return discretize(value, {step, total + step, 2*total + step}, 3);
}


char percent(short vect[], char idx, char len) {
	int i, total = 0, step;

	for (i = 0; i < 4; i++) {
		total += vect[i];
	}

	step = total / 4;

	return discretize(vect[idx], {step, 2*step, 3*step}, 3);
}
