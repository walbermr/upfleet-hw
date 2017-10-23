#include "abrasion.h"

short RPM_THRESHOLD[] = {1500, 2500, 3500};
short SPD_THRESHOLD[] = {6, 13, 20};
short BRK_THRESHOLD[] = {1000, 2000, 3000};

short RPM_RATE_THRESHOLD[] = {15, 30, 45};
short BRK_RATE_THRESHOLD[] = {16, 32, 64};

char BRAKE_WEAR[16]	= {	0x0, 0x0, 0x1, 0x2, 
						0x0, 0x1, 0x1, 0x2, 
						0x0, 0x1, 0x2, 0x3, 
						0x1, 0x1, 0x2, 0x3};

char CLUTCH_WEAR[8]	= {	0x0, 0x0, 
						0x1, 0x0, 
						0x2, 0x0, 
						0x3, 0x0};

char ENGINE_WEAR[16] = {0x0, 0x0, 0x0, 0x0,
						0x0, 0x0, 0x1, 0x1,
						0x0, 0x1, 0x1, 0x2,
						0x1, 0x2, 0x2, 0x3};

short CUMULATIVE_BRAKE[] = {0, 0, 0, 0};
short CUMULATIVE_CLUTCH[] = {0, 0, 0, 0};
short CUMULATIVE_RPM[] = {0, 0, 0, 0};

short last_brk, last_rpm;


void printhex(short *buf, char size)
{
	int sum = 0;
	for (int i = 0; i < size; i++)
	{
		//if (i > 0) printf(":");
		//printf("%04X", buf[i]);
		sum += buf[i];
	}

	//printf("\n");
	return;
}


char discretize(short value, short thresh[], char len) {
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
	char speed, has_brake, brake_rate, rpm_rate;
	char brake_vars_bits[] = {2, 2};
	char clutch_vars_bits[] = {2, 1};

	//printf("rpm: %d\nspd: %d\nbrk: %d\n", rpm, spd, brk);

	speed = discretize(spd, SPD_THRESHOLD, 3);
	brake_rate = rate(last_brk, brk, BRK_RATE_THRESHOLD);
	rpm_rate = rate(last_rpm, rpm, RPM_RATE_THRESHOLD);

	//printf("rpm_rate:%d\n", (rpm - last_rpm));
	has_brake = (brk > 500)? 1: 0;

	char brake_vars[] = {speed, brake_rate};
	char clutch_vars[] = {rpm_rate, has_brake};

	char brake_idx = verifyWear(brake_vars, brake_vars_bits, 2, BRAKE_WEAR);
	char clutch_idx = verifyWear(clutch_vars, clutch_vars_bits, 2, CLUTCH_WEAR);
	char rpm_idx = discretize(rpm, RPM_THRESHOLD, 3);

	// printf("brake_idx: %u\nclutch_idx: %u\nrpm_idx: %u\n", brake_idx, clutch_idx, rpm_idx);
	
	CUMULATIVE_BRAKE[brake_idx] += 1;
	CUMULATIVE_CLUTCH[clutch_idx] += 1;
	CUMULATIVE_RPM[rpm_idx] += 1;

	// printf("BRAKE: "); printhex(CUMULATIVE_BRAKE, 4);
	// printf("CLUTCH: "); printhex(CUMULATIVE_CLUTCH, 4);
	// printf("RPM: "); printhex(CUMULATIVE_RPM, 4);

	last_brk = brk;
	last_rpm = rpm;

	return;
}


void resetWear(char v_len) {
	short *v[] = {CUMULATIVE_RPM, CUMULATIVE_CLUTCH, CUMULATIVE_BRAKE};
	for(char i = 0; i < 3; i++)
	{
		for(char j = 0; j < v_len; j++)
			v[i][j] = 0;
	}

	return;
}


char rate(short x1, short x2, short vect[]) {
	char dx = discretize(x2-x1, vect, 3);

	return (dx >= 0)? dx: 0;
}


char average(short vect[], short weight[]) {
	char i;
	short total = 0, value = 0, step;

	for (i = 0; i < 4; i++) {
		vect[i] <<= weight[i];
		value += vect[i] * i;
		total += vect[i];
	}

	step = 3*total / 4;
	short v[] = {step, (short)(2*step), (short)(3*step)};
	return discretize(value, v, 3);
}


void wearData(unsigned char* data_ret) {
	char brake_wear, clutch_wear, engine_wear, rpm, rpm_time;
	char engine_vars_bits[] = {2, 2};
	short rpm_weight[] = {0, 0, 0, 0}, brake_weight[] = {0, 1, 5, 8}, clutch_weight[] = {0, 1, 5, 8};

	rpm = average(CUMULATIVE_RPM, rpm_weight);
	rpm_time = percent(CUMULATIVE_RPM, rpm, 4);
	
	char engine_vars[] = {rpm, rpm_time};

	brake_wear = average(CUMULATIVE_BRAKE, brake_weight);
	clutch_wear = average(CUMULATIVE_CLUTCH, clutch_weight);
	engine_wear = verifyWear(engine_vars, engine_vars_bits, 2, ENGINE_WEAR);

	data_ret[0] = (brake_wear << 4) + (clutch_wear << 2) + engine_wear;
	data_ret[1] = '\0';
}


char percent(short vect[], char idx, char len) {
	char i;
	short total = 0, step;

	for (i = 0; i < 4; i++) {
		total += vect[i];
	}

	step = total / 4;
	short v[] = {step, (short)(2*step), (short)(3*step)};
	return discretize(vect[idx], v, 3);
}
