#ifndef ABRASION_H
#define ABRASION_H

char discretize(int value, int thresh[], char len);
char verifyWear(char param[], char param_bits[], char n_param, char wear[]);
void accumulateWear(int engine, int clutch, int brake);
char percent(unsigned char vect[], char idx, char len);
char rate(char x1, char x2, int vect[]);
void resetWear(char v_len);

#endif // ABRASION_H
