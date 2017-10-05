#include "abrasion.h"

char discretize(int value, int thresh[], char len) {
	char out;

	for (out = 0; out < len; out++) {
		if (value <= thresh[out]) {
			break;
		}
	}

	return out;
}

char verifyWear(char high, char low, char low_bits, char wear[]) {
  char i = high << low_bits + low;

  return wear[i];
}
