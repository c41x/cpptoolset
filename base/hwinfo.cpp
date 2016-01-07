#include "hwinfo.hpp"
#include "string.hpp"
#include "timer.hpp"

#ifdef GE_COMPILER_VISUAL
#include <intrin.h>
#endif

namespace granite { namespace base {

namespace cpu {

namespace detail {
bool _fetched = false;
char _IDString[32];
char _BrandName[64];
int _CacheLineSize;
int _L2Associativity;
int _CacheSize;
bool _MMX;
bool _SSE;
bool _SSE2;
bool _AMD3DNow;
bool _AMD3DNowEx;
bool _MMXEx;
bool _CMOV;
bool _64BIT;

#ifdef GE_COMPILER_VISUAL
inline uint64 rdtsc() {
	return __rdtsc();
}

inline void cpuid(int o[4], int i) {
	__cpuid(o, i);
}
#endif

#ifdef GE_COMPILER_GCC
inline uint64 rdtsc() {
    uint64 x;
    __asm__ volatile (".byte 0x0f, 0x31" : "=A" (x));
    return x;
}

void cpuid(int o[4], int i) {
	o[0] = i;
	asm volatile("cpuid"
				 : "=a" (o[0]),
				   "=b" (o[1]),
				   "=c" (o[2]),
				   "=d" (o[3])
				 : "0" (o[0]), "2" (o[2]));
}
#endif

void fetch(bool force = false) {
	if (!_fetched || force) {
		// zapelnia dane
		int out[4];
		int maxinfo = 0, maxinfoex = 0;

		// poziom 0 (zawsze dostepny)
		cpuid(out, 0);
		maxinfo = out[0]; // maksymalna ilosc poziomow informacji dla __cpuid
		memset(_IDString, '\0', sizeof(_IDString));
		memcpy(_IDString, out, sizeof(out));

		// poziom 1 (niekoniecznie musi byc dostepny(jak wszystkie ponizej))
		if (maxinfo >= 1) {
			cpuid(out, 1);
			_CMOV = (out[3] & (1 << 15)) || false; // Conditional Move/Compare Instruction
			_MMX = (out[3] & (1 << 23)) || false; // MMX Technology
			_SSE = (out[3] & (1 << 25)) || false; // SSE Extensions
			_SSE2 = (out[3] & (1 << 26)) || false; // SSE2 Extensions
		}

		// sprawdzamy czy sa dane z poziomu rozszerzonego
		cpuid(out, 0x80000000);
		maxinfoex = out[0] - 0x80000000;

		// rozszerzony 1
		if (maxinfoex >= 1) {
			_AMD3DNow = (out[3] & (1 << 31)) || false; // 3Dnow! instructions (AMD) / Reserved (Intel)
			_AMD3DNowEx = (out[3] & (1 << 30)) || false; // 3DnowExt (AMD) / Reserved (Intel)
			_MMXEx = (out[3] & (1 << 22)) || false; // Extensions to MMX instructions (AMD) / Reserved (Intel)
			_64BIT = (out[3] & (1 << 23)) || false; // 64-bit Technology available
		}

		// rozszerzony 2
		if (maxinfoex >= 2) {
			cpuid(out, 0x80000002);
			memset(_BrandName, '\0', sizeof(_BrandName));
			memcpy(_BrandName, out, sizeof(out));

			//rozszerzony 3
			if (maxinfoex >= 3) {
				cpuid(out, 0x80000003);
				memcpy(_BrandName + 16, out, sizeof(out));

				//rozszerzony 4
				if (maxinfoex >= 4) {
					cpuid(out, 0x80000004);
					memcpy(_BrandName + 32, out, sizeof(out));
				}
			}

			// trim whitespaces
			int w = 0;
			for (uint i = 0; i < strlen(_BrandName); ++i) {
				if (isWhiteSpace(_BrandName[i]))
					w = i;
				else break;
			}
			strcpy(_BrandName, _BrandName + w + 1);
		}

		// rozszerzony 6
		if (maxinfoex >= 6) {
			cpuid(out, 0x80000006);
			_CacheLineSize = out[2] & 0xff; // Cache Line Size
			_L2Associativity = (out[2] >> 12) & 0xf; // L2 Associativity
			_CacheSize = (out[2] >> 16) & 0xffff; // Cache size in 1K units
		}
		_fetched = true;
	}
}
}

//- CPU info -
void refresh() { detail::fetch(true); }

string toStr() {
	detail::fetch();
	return strs(detail::_BrandName,
				" cache line size: ", detail::_CacheLineSize, "b ",
				detail::_CacheSize, "kb L2",
				detail::_64BIT ? " 64bit [ " : " [ ",
				detail::_MMX ? "MMX " : "",
				detail::_MMXEx ? "MMXEx " : "",
				detail::_SSE ? "SSE " : "",
				detail::_SSE2 ? "SSE2 " : "",
				detail::_AMD3DNow ? "AMD3DNow " : "",
				detail::_AMD3DNowEx ? "AMD3DNowEx " : "",
				detail::_CMOV ? "CMOV ]" : "]");
}

const char *getBrandName() { detail::fetch(); return detail::_BrandName; }
const char *getId() { detail::fetch(); return detail::_IDString; }
int getCacheSize() { detail::fetch(); return detail::_CacheSize; }
int getCacheLineSize() { detail::fetch(); return detail::_CacheLineSize; }
int getL2Associativity() { detail::fetch(); return detail::_L2Associativity; }
bool supportMMX() { detail::fetch(); return detail::_MMX; }
bool supportSSE() { detail::fetch(); return detail::_SSE; }
bool supportSSE2() { detail::fetch(); return detail::_SSE2; }
bool support3DNow() { detail::fetch(); return detail::_AMD3DNow; }
bool support3DNowEx() { detail::fetch(); return detail::_AMD3DNowEx; }
bool supportMMXEx() { detail::fetch(); return detail::_MMXEx; }
bool supportCMOV() { detail::fetch(); return detail::_CMOV; }
bool support64bit() { detail::fetch(); return detail::_64BIT; }

//- cycles measurment utility -
cycleTimer::cycleTimer() {}
cycleTimer::~cycleTimer() {}
void cycleTimer::reset() { _last = detail::rdtsc(); }
uint64 cycleTimer::elapsed() {
	uint64 curr = detail::rdtsc();
	uint64 ret = curr - _last;
	_last = curr;
	return ret;
}
}

}}
