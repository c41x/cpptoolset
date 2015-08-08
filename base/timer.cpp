#include "timer.hpp"
#include "string.hpp"
#include "log.hpp"

namespace granite { namespace base {

double timer::m_secsPerTick = 1.0;

bool timer::init(){
#ifdef GE_PLATFORM_WINDOWS
	LARGE_INTEGER freq;
	if(QueryPerformanceFrequency(&freq)) {
		m_secsPerTick = 1.0 / double(freq.QuadPart);
		//int64 freqi64 = int64(freq.QuadPart);
		//logInfo(strs("timer initialized, frequency: ", freqi64, "Hz"));
		return true;
	}
	else {
		m_secsPerTick = 1.0;
		logError("timer initialization failed, could not enable high resolution timer");
		// TODO: windows errors codes
		// TODO: add frequency to log
		// TODO: strings utilities
		// TODO: initialize once
		return false;
	}
#elif defined(GE_PLATFORM_LINUX)
	timespec ts;
	clock_getres(CLOCK_REALTIME, &ts);
	int64 freq = int64(ts.tv_sec) * int64(1000000000) + int64(ts.tv_nsec);
	m_secsPerTick = 1.0 / double(freq);
	logInfo(strs("timer initialized, resolution: ", freq, "ns"));
	return true;
#else
	m_secsPerTick = 0.000001;
	return true;
#endif
}

int64 timer::tick() {
#ifdef GE_PLATFORM_WINDOWS
	LARGE_INTEGER li;
	gassert(QueryPerformanceCounter(&li), "query time failed, unable to use QueryPerformanceCounter");
	QueryPerformanceCounter(&li);
	return li.QuadPart;
#elif defined(GE_PLATFORM_LINUX)
	timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	return int64(ts.tv_sec) * int64(1000000000) + int64(ts.tv_nsec);
#else
	timeval tv;
	gettimeofday(&tv, NULL);
	return int64(tv.tv_sec) * int64(1000000) + int64(tv.tv_usec);
#endif
}

}}
