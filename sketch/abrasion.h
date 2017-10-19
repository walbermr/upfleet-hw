#ifndef ABRASION_H
#define ABRASION_H

char discretize(short value, short thresh[], char len);
char verifyWear(char param[], char param_bits[], char n_param, char wear[]);
void accumulateWear(short rpm, short spd, short brk);
char percent(short vect[], char idx, char len);
char rate(short x1, short x2, short vect[]);
void resetWear(char v_len);
void wearData(unsigned char* data_ret);

#endif // ABRASION_H
