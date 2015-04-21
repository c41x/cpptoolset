#include <base/base.h>

using namespace granite;
using namespace granite::base;

string stateToStr(bool fail) { return fail ? "[fail] " : "[ok] "; }

int main(int argc, char **argv) {
	rng<> r;
	bool fail = false;
	const int numTests = 10000;

	// uint max test
	for (int i = 0; i < numTests; ++i) {
		uint maxRand = rand();
		uint rn = r.uint(maxRand);
		if (rn > maxRand) {
			//std::cout << rn << ", " << maxRand << std::endl;
			fail = true;
		}
	}
	std::cout << stateToStr(fail) << "uint(max)" << std::endl;

	// uint range test
	fail = false;
	for (int i = 0; i < numTests; ++i) {
		uint minRand = rand() / 2;
		uint maxRand = minRand + rand() / 3;
		uint rn = r.uint(minRand, maxRand);
		if (rn > maxRand || rn < minRand) {
			//std::cout << minRand << ", " << rn << ", " << maxRand << std::endl;
			fail = true;
		}
	}
	std::cout << stateToStr(fail) << "uint(min, max)" << std::endl;

	// int max
	fail = false;
	for (int i = 0; i < numTests; ++i) {
		int maxRand = rand();
		int rn = r.integer(maxRand);
		if (rn > maxRand) {
			std::cout << rn << ", " << maxRand << std::endl;
			fail = true;
		}
	}
	std::cout << stateToStr(fail) << "integer(max)" << std::endl;

	// int range
	fail = false;
	for (int i = 0; i < numTests; ++i) {
		int minRand = rand() / 2;
		int maxRand = minRand + rand() / 3;
		if ((rand() % 100) > 50) {
			minRand = -minRand;
			maxRand = -maxRand;
			std::swap(minRand, maxRand);
		}
		int rn = r.integer(minRand, maxRand);
		if (rn > maxRand || rn < minRand) {
			std::cout << minRand << ", " << rn << ", " << maxRand << std::endl;
			fail = true;
		}
	}
	std::cout << stateToStr(fail) << "integer(min, max)" << std::endl;

	// float clamp
	fail = false;
	for (int i = 0; i < numTests; ++i) {
		float rn = r.clamp();
		if (rn < 0.f || rn > 1.f) {
			//std::cout << minRand << ", " << rn << ", " << maxRand << std::endl;
			fail = true;
		}
	}
	std::cout << stateToStr(fail) << "float clamp()" << std::endl;

	// float max
	fail = false;
	for (int i = 0; i < numTests; ++i) {
		float maxRand = float(rand() % 999) / float(rand() % 100 + 1);
		float rn = r.real(maxRand);
		if (rn > maxRand) {
			std::cout << rn << ", " << maxRand << std::endl;
			fail = true;
		}
	}
	std::cout << stateToStr(fail) << "real(max)" << std::endl;

	// int range
	fail = false;
	for (int i = 0; i < numTests; ++i) {
		float minRand = float(rand() % 999) / float(rand() % 100 + 1);
		float maxRand = minRand + float(rand() % 99) / float(rand() % 100 + 1);
		float rn = r.real(minRand, maxRand);
		if (rn > maxRand || rn < minRand) {
			//std::cout << minRand << ", " << rn << ", " << maxRand << std::endl;
			fail = true;
		}
	}
	std::cout << stateToStr(fail) << "real(min, max)" << std::endl;

	// seed test
	fail = false;
	uint32 rn[100];
	for (int i = 0; i < 100; ++i) {
		r.seed((uint32)i);
		for (int j = 0; j < 100; ++j) {
			rn[j] = r.uniform();
		}

		r.seed((uint32)i);
		for (int j = 0; j < 100; ++j) {
			if (rn[j] != r.uniform())
				fail = true;
		}
	}
	std::cout << stateToStr(fail) << "seed test" << std::endl;

	return 0;
}
