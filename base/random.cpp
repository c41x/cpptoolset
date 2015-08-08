#include "random.hpp"
#include "math.hpp"

namespace granite { namespace base{

float perlin::gradient2(int hash, float x, float y) {
	/**
	   kombinacja roznych gradientow
	   uzaleznionych od zmiennej hash, i bezposrednio
	   od indeksow x i y
	*/
	switch (hash & 15) {
		case 0x0: return x;
		case 0x1: return -x;
		case 0x2: return y;
		case 0x3: return -y;
		case 0x4: case 0xb: return x + y;
		case 0x5: case 0xc: return x - y;
		case 0x6: case 0xd: return -x - y;
		case 0x7: case 0xe: return y - x;
		case 0x8: case 0xf: return -y - x;
		case 0x9: return x + x;
		case 0xa: return y + y;
	}
	return 0.f;
}

int perlin::gethash2(int x, int y) {
	int a = perms[x & rsizem] + y;
	int b = perms[(x & rsizem) + 1] + y;
	return perms[(a + b) & rsizem];
}

float perlin::noise2(float x, float y, bool repeat, float frequency) {
	// indeksy w tablicy szumu (tablica liczb calkowitych do wyliczania gradientow)
	int iL = int(x);
	int iB = int(y);
	int iT = iB + 1;
	int iR = iL + 1;

	// powtarzanie tekstury (jesli czestotliwosc jest rowna indeksowi prawego lub dolnego brzegu
	// to podmmieniamy indeks odpowiednio: prawy i dolny na 0 - czyli poczatek tekstury
	// cos w rodzaju noise(xmax)==noise(0). czemu czestotliwosc? - bo ona jest rowna indeksowi
	// najbardziej 'prawego' i 'dolnego' indeksu tablicy szumu - WYMIAR
	if (repeat) {
		if (int(frequency) == iR) iR = 0;
		if (int(frequency) == iT) iT = 0;
	}

	// wspolrzedne interpolacji (wektory od lewego dolnego brzegu do srodka interpolacji)
	x -= iL;
	y -= iB;

	// pobiera 4 probki dla kazdego brzegu
	int v00 = gethash2(iL, iB);
	int v10 = gethash2(iR, iB);
	int v01 = gethash2(iL, iT);
	int v11 = gethash2(iR, iT);

	// oblicza dla nich gradienty na podstawie wektorow od wierzcholkow do srodka interpolacji
	// i oczywiscie pewnego parametru charakterystycznego dla wspolrzednych x,y (tablica hash)
	float g00 = gradient2(perms[v00], x, y);
	float g10 = gradient2(perms[v10], x - 1.f, y);
	float g01 = gradient2(perms[v01], x, y - 1.f);
	float g11 = gradient2(perms[v11], x - 1.f, y - 1.f);

	// zwracamy wynik interpolacji (kubiczna interpolacja 2d)
	return cubicInterp2d(g00, x, g10,
						 g01, y, g11);
}

float perlin::gradient1(int hash, float x) {
	switch (hash & 15) {
		case 0x0: case 0x1: case 0x2: case 0x3: return x;
		case 0x4: case 0xb: return -1.f;
		case 0x5: case 0xc: return 1.f;
		case 0x6: case 0xd: case 0x7: case 0xe: case 0x8: case 0xf: return -x;
		case 0x9: return x + x;
		case 0xa: return x;
	}
	return 0.f;
}

int perlin::gethash1(int x) {
	int a = perms[x & rsizem];
	int b = perms[(x & rsizem) + 1];
	return perms[(a + b) & rsizem];
}

float perlin::noise1(float x, bool repeat, float frequency) {
	int iL = int(x);
	int iR = iL + 1;
	if (repeat) if (int(frequency) == iR) iR=0;
	x -= iL;
	int v00 = gethash1(iL);
	int v10 = gethash1(iR);
	float g00 = gradient1(perms[v00], x);
	float g10 = gradient1(perms[v10], x - 1.f);
	return cubicInterp(g00,x,g10);
}

float perlin::gradient3(int hash, float x, float y, float z) {
	switch (hash & 15) {
		case 0x0: return x - z;
		case 0x1: return -x - z;
		case 0x2: return y + z;
		case 0x3: return -y - x;
		case 0x4: return z + y;
		case 0xb: return -z - x;
		case 0x5: return x + y + z;
		case 0xc: return x + y - z;
		case 0x6: return x - y - z;
		case 0xd: return -x - y - z;
		case 0x7: return -x + y + z;
		case 0xe: return -x - y + z;
		case 0x8: return -x + y - z;
		case 0xf: return -x + y - z;
		case 0x9: return x + y;
		case 0xa: return x + z;
	}
	return 0.f;
}

int perlin::gethash3(int x, int y, int z) {
	int a = perms[x & rsizem] + z;
	int b = perms[(y & rsizem) + 1] + z;
	return perms[(a + b) & rsizem];
}

float perlin::noise3(float x, float y, float z, bool repeat, float frequency) {
	int iL = int(x);
	int iB = int(y);
	int iN = int(z); // near
	int iT = iB + 1;
	int iR = iL + 1;
	int iF = iN + 1; // far

	if (repeat) {
		if (int(frequency) == iR) iR = 0;
		if (int(frequency) == iT) iT = 0;
		if (int(frequency) == iF) iF = 0;
	}

	// wspolrzedne interpolacji (wektory od lewego dolnego brzegu do srodka interpolacji)
	x -= iL;
	y -= iB;
	z -= iN;

	// pobiera 4 probki dla kazdego brzegu
	int vn00 = gethash3(iL, iB, iN);
	int vn10 = gethash3(iR, iB, iN);
	int vn01 = gethash3(iL, iT, iN);
	int vn11 = gethash3(iR, iT, iN);
	int vf00 = gethash3(iL, iB, iF);
	int vf10 = gethash3(iR, iB, iF);
	int vf01 = gethash3(iL, iT, iF);
	int vf11 = gethash3(iR, iT, iF);

	// oblicza dla nich gradienty na podstawie wektorow od wierzcholkow do srodka interpolacji
	// i oczywiscie pewnego parametru charakterystycznego dla wspolrzednych x,y,z (tablica hash)
	float gn00 = gradient3(perms[vn00], x, y, z);
	float gn10 = gradient3(perms[vn10], x - 1.f, y, z);
	float gn01 = gradient3(perms[vn01], x, y - 1.f, z);
	float gn11 = gradient3(perms[vn11], x - 1.f, y - 1.f, z);
	float gf00 = gradient3(perms[vf00], x, y, z - 1.f);
	float gf10 = gradient3(perms[vf10], x - 1.f, y, z - 1.f);
	float gf01 = gradient3(perms[vf01], x, y - 1.f, z - 1.f);
	float gf11 = gradient3(perms[vf11], x - 1.f, y - 1.f, z - 1.f);

	// cubic interp 3d
	return cubicInterp(cubicInterp2d(gn00, x, gn10,
									 gn01, y, gn11),
					   z,
					   cubicInterp2d(gf00, x, gf10,
									 gf01,y,gf11));
}

perlin::perlin() : perms(NULL) {
	init();
}

perlin::~perlin() {
	if (perms)
		delete perms;
}

void perlin::init() {
	/**
	   zapelnia tablice permutacji wartosciami o maksymalnej wartosci rownej
	   wymiarowi tablicy
	*/

	rsize = 512;
	rsizem = rsize - 1;
	if (perms == NULL)
		perms = new int[rsize];

	// wypelnia kolejno swoimi indeksami
	for (int i = 0; i < rsize; ++i)
		perms[i] = i;

	randomize();
}

void perlin::seed(uint32 _seed) {
	rg.seed(_seed);
}

void perlin::randomize() {
	amin = amax = 0.f;

	// shuffle
	for (int i = 0; i < rsize; ++i) {
		int tmp = perms[i], j = rg.uniform() & rsizem;
		perms[i] = perms[j];
		perms[j] = tmp;
	}
}

float perlin::fbm2(float X, float Y, float iPersistence, int iOctaves, bool iTiled) {
	/**
	   podaje wartosc szumu zlozonego z okreslonej liczby oktaw,
	   za kazda oktawa zwiekszajac czestotliwosc (coraz drobniejszy szum)
	   i zmniejszajac amplitude (drobniejszy szum bedzie mniej istotny) - czyli szum rozowy
	   zastosowano trick na usuniecie powtarzalnosci kazdej oktawy odwracajac
	   ja co 2 oktawe o 90 stopni (zamieniajac indeksy x z y)
	   zalecane wartosci parametrow:
	   x,y - od 0 do 1
	   iPersistence - 'wytrwalosc' od 0-1 (0 - 'gruby', 1 - 'drobny')
	   iOctaves - 4-15 - szczegolowosc (4 - 'gladki' >4 - 'poszarpany')
	*/

	float sum = 0.f;
	float freq = 1.f;
	float amp = 1.f;
	for (int i = 0; i < iOctaves; ++i) {
		sum += amp * (i & 1 ?
					  noise2(X * freq, Y * freq, iTiled, freq) :
					  noise2(Y * freq, X * freq, iTiled, freq));
		freq *= 2.f;
		amp *= iPersistence;
	}
	return sum;
}

void perlin::fbm2(float *oResult, int iWidth, int iHeight, float iPersistence, int iOctaves, bool iTiled) {
	float px, py;
	amax = -999.f;
	amin = 999.f;
	int ind = 0;
	for (int y = 0; y < iHeight; ++y)
		for(int x = 0; x < iWidth; ++x, ++ind)
		{
			px = float(x) / float(iWidth);
			py = float(y) / float(iHeight);
			oResult[ind] = fbm2(px, py, iPersistence, iOctaves, iTiled);

			if (oResult[ind] > this->amax)
				this->amax = oResult[ind];
			if (oResult[ind] < this->amin)
				this->amin = oResult[ind];
		}
}

float perlin::fbm1(float X, float iPersistence, int iOctaves, bool iTiled) {
	float sum = 0.f;
	float freq = 1.f;
	float amp = 1.f;
	for (int i = 0; i < iOctaves; ++i) {
		sum += amp * noise1(X * freq, iTiled, freq);
		freq *= 2.f;
		amp *= iPersistence;
	}
	return sum;
}

void perlin::fbm1(float *oResult, int iWidth, float iPersistence, int iOctaves, bool iTiled) {
	float px;
	amax = -999.f;
	amin = 999.f;
	for (int x = 0; x < iWidth; ++x) {
		px = float(x) / float(iWidth);
		oResult[x] = fbm1(px, iPersistence, iOctaves, iTiled);

		if (oResult[x] > this->amax)
			this->amax = oResult[x];
		if (oResult[x] < this->amin)
			this->amin = oResult[x];
	}
}

float perlin::fbm3(float X, float Y, float Z, float iPersistence, int iOctaves, bool iTiled) {
	float sum = 0.f;
	float freq = 1.f;
	float amp = 1.f;
	for (int i = 0; i < iOctaves; ++i) {
		sum += amp * (i & 1 ?
					  noise3(X * freq, Y * freq, Z * freq, iTiled, freq) :
					  noise3(Y * freq, X * freq, Z * freq, iTiled, freq));
		freq *= 2.f;
		amp *= iPersistence;
	}
	return sum;
}

void perlin::fbm3(float *oResult, int iWidth, int iHeight, int iDepth, float iPersistence, int iOctaves, bool iTiled) {
	float px, py, pz;
	amax = -999.f;
	amin = 999.f;
	int ind = 0;
	for (int z = 0; z < iDepth; ++z)
		for (int y = 0; y < iHeight; ++y)
			for (int x = 0; x < iWidth; ++x, ++ind) {
				px = float(x) / float(iWidth);
				py = float(y) / float(iHeight);
				pz = float(z) / float(iDepth);
				oResult[ind]= fbm3(px, py, pz, iPersistence, iOctaves, iTiled);

				if (oResult[ind] > this->amax)
					this->amax = oResult[ind];
				if (oResult[ind] < this->amin)
					this->amin = oResult[ind];
			}
}

std::tuple<float, float> perlin::getAmplitude() const {
	return std::make_tuple(amin, amax);
}

}}
//~
