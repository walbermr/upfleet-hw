#ifndef ABRASION_H
#define ABRASION_H

char discretize(int value, int thresh[], char len);
char verifyWear(char high, char low, char low_bits, char wear[]);
void accumulateWear(int engine, int clutch, int brake);
void resetWear(char v_len);

#endif // ABRASION_H
