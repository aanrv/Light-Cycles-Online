#include "h.h"
#include <stdlib.h>

unsigned short strtoport(char* str) {
	const int BASE = 10;		// conversion base
	long out = strtol(str, NULL, BASE);

	if (out < PORTMIN || out > PORTMAX) out = 0;
	return (unsigned short) out;
}
