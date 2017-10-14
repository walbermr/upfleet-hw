#ifndef ABRASION_H
#define ABRASION_H

char discretize(short value, short thresh[], char len);
char verifyWear(short param[], char param_bits[], char n_param, char wear[]);
void accumulateWear(short rpm, short spd, short brk);
char percent(unsigned char vect[], char idx, char len);
char rate(char x1, char x2, short vect[]);
void resetWear(char v_len);
void wearData(char* data_ret);

#endif // ABRASION_H
