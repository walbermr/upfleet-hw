#include "abrasion.h"

short RPM_THRESHOLD[] = {500, 1500, 2000};
short SPD_THRESHOLD[] = {2000, 3000, 4500};
short BRK_THRESHOLD[] = {1000, 2000, 3000};

char BRAKE_WEAR[36]	= {};
char CLUTCH_WEAR[8]	= {};
char ENGINE_WEAR[16]	= {};

unsigned char CUMULATIVE_BRAKE[] = {0, 0, 0, 0};
unsigned char CUMULATIVE_CLUTCH[] = {0, 0, 0, 0};
unsigned char CUMULATIVE_RPM[] = {0, 0, 0, 0};

char last_brk, last_rpm;


char discretize(short value, short thresh[], char len) {
	char out;

	for (out = 0; out < len; out++) {
		if (value <= thresh[out]) {
			break;
		}
	}

	return out;
}


char verifyWear(short param[], char param_bits[], char n_param, char wear[]) {
	char i, in = 0;

	for (i = 0; i < n_param; i++) {
		in <<= param_bits[i];
		in += param[i];
	}

	return wear[in];
}


void accumulateWear(short rpm, short spd, short brk) {	//acumula valores de desgaste
	char speed, has_brake, brake_rate, rpm_rate;

	short brake_vars[] = {speed, brake_rate};
	char brake_vars_bits[] = {2, 2};

	short clutch_vars[] = {rpm_rate, has_brake};
	char clutch_vars_bits[] = {2, 1};

	speed = discretize(spd, SPD_THRESHOLD, 3);
	brake_rate = rate(last_brk, brk, BRK_THRESHOLD);

	rpm_rate = rate(last_rpm, rpm, RPM_THRESHOLD);
	has_brake = brk > 0;

	CUMULATIVE_BRAKE[verifyWear( brake_vars, brake_vars_bits, 2, BRAKE_WEAR)] += 1;
	CUMULATIVE_CLUTCH[verifyWear( clutch_vars, clutch_vars_bits, 2, CLUTCH_WEAR)] += 1;
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


char rate(char x1, char x2, short vect[]) {
	char dx = discretize(x2, vect, 3) - discretize(x1, vect, 3);

	return (dx >= 0)? dx: -dx;
}


char average(unsigned char vect[]) {
	char i;
	short total = 0, value = 0, step;

	for (i = 0; i < 4; i++) {
		value += vect[i] * i;
		total += vect[i];
	}

	step = total / 2;
	short v[] = {step, total + step, 2*total + step};
	return discretize(value, v, 3);
	}


void wearData(char* data_ret) {
	char brake_wear, clutch_wear, engine_wear, rpm, rpm_time;
	short engine_vars[] = {rpm, rpm_time};
	char engine_vars_bits[] = {2, 2};

	rpm = average(CUMULATIVE_RPM, 4);
	rpm_time = percent(CUMULATIVE_RPM, rpm, 4);

	brake_wear = average(CUMULATIVE_BRAKE, 4);
	clutch_wear = average(CUMULATIVE_CLUTCH, 4);
	engine_wear = verifyWear(engine_vars, engine_vars_bits, 2, ENGINE_WEAR);

	data_ret[0] = brake_wear << 4 + clutch_wear << 2 + engine_wear;
	data_ret[1] = '\0';
}


char average(unsigned char vect[]) {
	char i;
	short total = 0, value = 0, step;

	for (i = 0; i < 4; i++) {
		value += vect[i] * i;
		total += vect[i];
	}

	step = total / 2;
	short v[] = {step, total + step, 2*total + step};
	return discretize(value, v, 3);
}


char percent(unsigned char vect[], char idx, char len) {
	char i;
	char total = 0, step;

	for (i = 0; i < 4; i++) {
		total += vect[i];
	}

	step = total / 4;
	short v[] = {step, 2*step, 3*step};
	return discretize(vect[idx], v, 3);
}
