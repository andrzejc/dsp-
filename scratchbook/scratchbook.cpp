// scratchbook.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

struct is_right_shift_arithmetic {enum {value = (0 != ((INT_MIN >> 1) & INT_MIN))};};

template<class Signed> Signed ars(Signed v, int sh) {
	if (v >= 0 || sh <= 0)
		return v >> sh;
	else if (sh >= std::numeric_limits<Signed>::digits)
		return Signed(-1); // shifting any negative number more than nbits - 1 will always yield -1
	else {
		while (sh != 0) {
			Signed odd = v & 1;
			v /= 2;
			if (odd)
				--v;
			--sh;
		}
		return v;
	}
}

void test_right_shift() {
	for (int i = SHRT_MIN; i <= SHRT_MAX; ++i) {
		for (short j = 0; j <= 17; ++j) {
			short s = short(i);
			short ref = s >> j; 
			short tes = ars(s, j);
			if (ref != tes) {
				printf("ars failed for %d >> %d; exp %d got %d\n", i, (int)j, (int)ref, (int)tes);
			}
		}
	}
}

int main(int argc, char* argv[])
{
	int r = -15 % 7;
	test_right_shift();

	return 0;
}

