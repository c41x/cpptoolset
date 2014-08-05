/*
 * granite engine 1.0 | 2006-2013 | Jakub Duracz | jakubduracz@gmail.com | http://jakubduracz.com
 * file: timer.*
 * created: 19-01-2013
 *
 * description: high resolution timers (1ns resolution)
 *
 * changelog:
 * - 23-09-2006: original timer in old engine
 * - 19-01-2013: file created
 * - 22-01-2013: reset function added
 *
 * notes:
 * - on linux platform you may need to link librt.a/librt.so - just add -lrt in g++ commandline
 */

#pragma once

#include "includes.h"

namespace granite { namespace base {

class timer {
	int64 m_start;
	static double m_secsPerTick;

public:
	timer() { m_start = tick(); }
	~timer(){}

	// statics
	static bool init();
	static int64 tick();
	static int64 delta(const int64 &t1,const int64 &t2) { return t2-t1; }
	static inline double deltaS(const int64 &t1, const int64 &t2);
	static inline double deltaMs(const int64 &t1, const int64 &t2);
	static inline double deltaUs(const int64 &t1, const int64 &t2);
	static inline double deltaNs(const int64 &t1, const int64 &t2);

	// get/set start time
	void reset() { m_start = tick(); }
	void setStartTick(const int64 &iTick) { m_start = iTick; }
	int64 getStartTick()const { return m_start; }

	// reset start time and get result time
	int64 time() { int64 t = tick(); ret = delta(m_start, t); m_start = t; return ret; }
	double timeS() { int64 t = tick(); double ret = deltaS(m_start, t); m_start = t; return ret; }
	double timeMs() { int64 t = tick(); double ret = deltaMs(m_start, t); m_start = t; return ret; }
	double timeUs() { int64 t = tick(); double ret = deltaUs(m_start, t); m_start = t; return ret; }
	double timeNs() { int64 t = tick(); double ret = deltaNs(m_start, t); m_start = t; return ret; }

	// just give me time without changing start time
	int64 getTime() const { return delta(m_start, tick()); }
	double getTimeS() const { return deltaS(m_start, tick()); }
	double getTimeMs() const { return deltaMs(m_start, tick()); }
	double getTimeUs() const { return deltaUs(m_start, tick()); }
	double getTimeNs() const { return deltaNs(m_start, tick()); }
};

#if defined(GE_PLATFORM_WINDOWS)
double timer::deltaS(const int64 &t1, const int64 &t2) { return double(t2 - t1) * m_secsPerTick; }
double timer::deltaMs(const int64 &t1, const int64 &t2) { return double(t2 - t1) * m_secsPerTick * 1000.0; }
double timer::deltaUs(const int64 &t1, const int64 &t2) { return double(t2 - t1) * m_secsPerTick * 1000000.0; }
double timer::deltaNs(const int64 &t1, const int64 &t2) { return double(t2 - t1) * m_secsPerTick * 1000000000.0; }
#else
double timer::deltaS(const int64 &t1, const int64 &t2) { return double(t2 - t1); }
double timer::deltaMs(const int64 &t1, const int64 &t2) { return double(t2 - t1) * 0.001; }
double timer::deltaUs(const int64 &t1, const int64 &t2) { return double(t2 - t1) * 0.000001; }
double timer::deltaNs(const int64 &t1, const int64 &t2) { return double(t2 - t1) * 0.000000001; }
#endif

}}
