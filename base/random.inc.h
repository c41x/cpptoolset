//- SHR generator -
void generatorSHR::seed(uint32 iSeed) {
	_seed = iSeed;
}

uint32 generatorSHR::uniform() {
	_seed ^= (_seed << 17);
	_seed ^= (_seed >> 13);
	_seed ^= (_seed << 5);
	return _seed;
}

//- MCS generator
void generatorMCS::seed(uint32 iSeed) {
	_seed = _seed2 = _seed3 = _seed4 = iSeed;
}

uint32 generatorMCS::uniform() {
	_seed ^= (_seed << 17);
	_seed ^= (_seed >> 13);
	_seed ^= (_seed << 5);
	_seed2 = 36969 * (_seed2 & 0xffffu) + (_seed2 >> 16);
	_seed3 = 18000 * (_seed3 & 0xffffu) + (_seed3 >> 16);
	_seed4 = 69069 * _seed4 + 1325476;
	return ((_seed2 << 16) + _seed3) ^ ((_seed4) + (_seed));
}

//- generator base -
template <typename T_GENERATOR> rng<T_GENERATOR>::rng() { _gen.seed(static_cast<uint32>(timer::tick())); }
template <typename T_GENERATOR> rng<T_GENERATOR>::rng(uint32 iSeed) { _gen.seed(iSeed); }
template <typename T_GENERATOR> rng<T_GENERATOR>::~rng() {}
template <typename T_GENERATOR> void rng<T_GENERATOR>::seed(uint32 iSeed) { _gen.seed(iSeed); }
template <typename T_GENERATOR> uint32 rng<T_GENERATOR>::uniform() { return _gen.uniform(); }
template <typename T_GENERATOR> uint32 rng<T_GENERATOR>::uint(uint32 iMax) { return uniform() % (iMax+1); }
template <typename T_GENERATOR> uint32 rng<T_GENERATOR>::uint(uint32 iMin, uint32 iMax) { return iMin + uniform() % (iMax - iMin + 1); }
template <typename T_GENERATOR> int32 rng<T_GENERATOR>::integer(int32 iMax) {
	return static_cast<int32>(uint(static_cast<uint32>(iMax < 0 ? - iMax : iMax) + 1)) * ((iMax < 0) ? (-1) : (1));
}
template <typename T_GENERATOR> int32 rng<T_GENERATOR>::integer(int32 iMin, int32 iMax) { return static_cast<int32>(uint(iMax - iMin + 1)) + iMin; }
template <typename T_GENERATOR> bool rng<T_GENERATOR>::boolean() { return uniform() > 0x7ffffffful; }
template <typename T_GENERATOR> float rng<T_GENERATOR>::clamp() { return (float(uniform()) / float(0xfffffffful)); }
template <typename T_GENERATOR> float rng<T_GENERATOR>::real(float iMin, float iMax) { return (iMax - iMin) * clamp() + iMin; }
template <typename T_GENERATOR> float rng<T_GENERATOR>::real(float iMax) { return clamp() * iMax; }
template <typename T_GENERATOR> template<typename T> T &rng<T_GENERATOR>::select(T &container, size_t size) { return container[uint(size)]; }
template <typename T_GENERATOR> template<typename T> T rng<T_GENERATOR>::select(const T &a, const T &b) { if(boolean()) return b; return a; }
