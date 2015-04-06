/*
 * granite engine 1.0 | 2006-2015 | Jakub Duracz | jakubduracz@gmail.com | http://jakubduracz.com
 * file: random
 * created: 16-12-2008
 *
 * description: random - pseudo random number generator with period of 2^130 for 32bit numbers,
 *              also contains some noise generators
 *
 * changelog:
 * - 16-12-2008: file created
 * - 15-01-2011: seed in constructor
 * - 12-02-2011: seed for perlin generators (for AutomatedLevelDesign dompo)
 * - 04-04-2015: refactor
 */

#pragma once
#include "includes.h"
#include "timer.h"

namespace granite { namespace base {

//- basic random generators -
// SHR generator
struct generatorSHR {
	uint32 _seed; // basic seed (SHR)
	inline void seed(uint32 iSeed);
	inline uint32 uniform();
};

// MWC^CONG+SHR generator
struct generatorMCS {
	uint32 _seed; // basic seed (SHR)
	uint32 _seed2; // i kilka dodatkowych (MWC)
	uint32 _seed3; // MWC
	uint32 _seed4; // CONG
	inline void seed(uint32 iSeed);
	inline uint32 uniform();
};

//- basic radom generator class -
template <typename T_GENEREATOR = generatorMCS>
class rng
{
private:
	T_GENEREATOR _gen;

public:
	rng();
	explicit rng(uint32 iSeed);
	~rng();

	inline void seed(uint32 iSeed);

	// basic
	inline uint32 uniform();

	// higher level
	inline uint32 uint(uint32 iMax);
	inline uint32 uint(uint32 iMin, uint32 iMax);
	inline int32 integer(int32 iMax);
	inline int32 integer(int32 iMin, int32 iMax);
	inline bool boolean();
	inline float clamp();
	inline float real(float iMin, float iMax);
	inline float real(float iMax);
	template<typename T> inline T &select(T &container, size_t size);
	template<typename T> inline T select(const T &a, const T &b);
};

class perlin
{
private:
	rng<> rg;
	int *perms; // permutation table
	int rsize, rsizem; // perms size, perms size - 1
	float amin, amax; // amplitude

	float gradient2(int hash, float x, float y);
	int gethash2(int x, int y);
	float gradient1(int hash, float x);
	int gethash1(int x);
	float gradient3(int hash, float x, float y, float z);
	int gethash3(int x, int y, int z);

public:
	perlin();
	~perlin();

	void seed(uint32 iSeed);
	void init();
	void randomize();

	float noise1(float x, bool repeat, float frequency);
	float noise2(float x, float y, bool repeat, float frequency);
	float noise3(float x, float y, float z, bool repeat, float frequency);
	float fbm2(float X, float Y, float iPersistence, int iOctaves, bool iTiled);
	void fbm2(float *oResult, int iWidth, int iHeight, float iPersistance, int iOctaves, bool iTiled);
	float fbm1(float X, float iPersistence, int iOctaves, bool iTiled);
	void fbm1(float *oResult, int iWidth, float iPersistance, int iOctaves, bool iTiled);
	float fbm3(float X, float Y, float Z, float iPersistence, int iOctaves, bool iTiled);
	void fbm3(float *oResult, int iWidth, int iHeight, int iDepth, float iPersistance, int iOctaves, bool iTiled);

	std::tuple<float, float> getAmplitude() const;
};

#include "random.inc.h"

}}

//~
