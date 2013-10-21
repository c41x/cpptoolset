/*
 * granite engine 1.0 | 2006-2012 | Jakub Duracz | jakubduracz@gmail.com | http://jakubduracz.com 
 * file: math.*
 * created: 22-10-2012
 * 
 * description: math module based on SIMD
 * 
 * changelog:
 * - 12-09-2006: initial add date in old engine version
 * - 22-10-2012: add
 * - 04-12-2012: sse vector class finished
 * - 08-12-2012: added basic constructors & operators for vec2f, vec3f, vec4f
 * - 09-12-2012: fixed some strange bug (?) in visual studio (see notes: "VS Bug")
 * - 09-12-2012: replaced mm_store_ss to mm_cvtss_f32 (it resolves VS "bug")
 * - 09-12-2012: added functions equal (to test if given float/double are "equal")
 * - 09-12-2012: added fast shuffle method to vec class
 * - 10-12-2012: templated min/max
 * - 21-01-2013: added helper and interpolations functions
 * 
 * notes:
 * - SSE data alignment is little-endian so conversion from/to SSE vec are done in reverse order
 * - SSE is required, there are no falloff versions of SSE classes/functions
 * - VS BUG: in visual studio compiler function _mm_store_ss must have precomputed variable in
 *   second argument (xmm) eg. sth like that fails: _mm_store_ss(&ret,_mm_sqrt_ps(lengthSq()));
 *   and ret will always be equal 0 - WTF? but this works as expected:
 *   __m128 t=_mm_sqrt_ps(lengthSq()); _mm_store_ss(&ret,t);
 * - lost 2 hours on "multiple declaration of" function linker error :| function in header
 *   must be inline... at 1am my brain is not working properly
 */

#pragma once

#include "includes.h"

// reverse shuffle
#define SSE_RSHUFFLE(VX, VY, VZ, VW) _MM_SHUFFLE(VW, VZ, VY, VX)

namespace granite {
namespace base {

// some consts
const float GE_E=2.71828182845904523536f; // e
const float GE_LOG2E=1.44269504088896340736f; // log2(e)
const float GE_LOG10E=0.434294481903251827651f; // log(e)
const float GE_LN2=0.693147180559945309417f; // ln(2)
const float GE_LN10=2.30258509299404568402f; // ln(10)
const float GE_PI=3.14159265358979323846f; // pi
const float GE_2PI=6.28318530717958647692f; // 2.f*pi
const float GE_PI_D2=1.57079632679489661923f; // pi/2.f
const float GE_PI_D4=0.785398163397448309616f; // pi/4.f
const float GE_1D_PI=0.318309886183790671538f; // 1.f/pi
const float GE_2D_PI=0.636619772367581343076f; // 2.f/pi
const float GE_180D_PI=57.295779513082320876798154814105f; // 180.f/pi
const float GE_PID_180=0.017453292519943295769236907684886f; // pi/180.f
const float GE_2D_SQRTPI=1.12837916709551257390f; // 2.f/sqrt(pi)
const float GE_SQRT2=1.41421356237309504880f; // sqrt(2)
const float GE_1D_SQRT2=0.707106781186547524401f; // 1.f/sqrt(2)

// functions
inline bool equal(float a,float b){
	return std::abs(a-b)<=1e-5;
}

inline bool equal(double a,double b,double ep=1e-13){
	return std::abs(a-b)<=ep;
}

template<typename T>inline T clip(const T &imin,const T &ix,const T &imax){
	if(ix<imin)
		return imin;
	if(ix>imax)
		return imax;
	return ix;
}

inline float clamp(float imin,float ix,float imax){
	return (ix-imin)/(imax-imin);
}

inline float stretch(float ileft,float ix,float iright,float imin,float imax){
	return imin+clamp(ileft,ix,iright)*(imax-imin);
}
	
// interpolations:
inline float linearInterp(float imin,float ix,float imax){
	return (imax-imin)*ix+imin;
}

inline float linearInterp2d(float ileft,float ix,float iright,float itop,float iY,float ibottom){
	return linearInterp(linearInterp(ileft,ix,iright),iY,linearInterp(itop,ix,ibottom));
}

inline float cubicInterp(float imin,float ix,float imax){
	return (imax-imin)*((3.f-2.f*ix)*ix*ix)+imin;
}

inline float cubicInterp2d(float ileft,float ix,float iright,float itop,float iY,float ibottom){
	return cubicInterp(cubicInterp(ileft,ix,iright),iY,cubicInterp(itop,ix,ibottom));
}

inline float cosineInterp(float imin,float ix,float imax){
	float T=(1.f-cosf(ix*GE_PI))*.5f;
	return (imax-imin)*T+imin;
}

inline float cosineInterp2d(float ileft,float ix,float iright,float itop,float iY,float ibottom){
	return cosineInterp(cosineInterp(ileft,ix,iright),iY,cosineInterp(itop,ix,ibottom));
}

inline float fadeInterp(float imin,float ix,float imax){
	float t=ix*ix*ix*(ix*(ix*6.f-15.f)+10.f);
	return (imax-imin)*t+imin;
}

inline float fadeInterp2d(float ileft,float ix,float iright,float itop,float iY,float ibottom){
	return fadeInterp(fadeInterp(ileft,ix,iright),iY,fadeInterp(itop,ix,ibottom));
}

inline float hyperbInterp(float imin,float ix,float imax){
	return (imax-imin)*(-1.f/(float(ix)-1.61f)-.61f)+imin;
}

inline float hyperbInterp2d(float ileft,float ix,float iright,float itop,float iY,float ibottom){
	return hyperbInterp(hyperbInterp(ileft,ix,iright),iY,hyperbInterp(itop,ix,ibottom));
}

template<typename T>inline bool isPow2(T ix){
	return !(ix&(ix-1));
}

template<typename T>T greaterEqualPow2(T ix){
	--ix;
	T w=1<<1; //2
	while(ix>>=1)
		w<<=1;
	return w;
}

template<typename T>T greaterPow2(T ix){
	T w=1<<1; //2
	while(ix>>=1)
		w<<=1;
	return w;
}

inline float degToRad(float ix){
	return GE_PID_180*ix;
}

inline float radToDeg(float ix){
	return GE_180D_PI*ix;
}

// non optimized float vectors
class vec2f {
public:
	GE_ALIGN_BEGIN(16) union{
		struct{
			float x,y;
		};
		float data[2];
	}GE_ALIGN_END(16);
	vec2f(const float _x,const float _y):x(_x),y(-y){}
	vec2f(const float _v):x(_v),y(_v){}
	vec2f(const float *p){x=*p;y=*(p+1);}	
	vec2f(){}
	~vec2f(){}
	vec2f &operator+=(const vec2f &v){x+=v.x;y+=v.y;return *this;}
	vec2f &operator-=(const vec2f &v){x-=v.x;y-=v.y;return *this;}
	vec2f &operator*=(const float v){x*=v;y*=v;return *this;}
	vec2f &operator*=(const vec2f &r){x*=r.x;y*=r.y;return *this;}
	vec2f &operator/=(const float v){x/=v;y/=v;return *this;}
	vec2f &operator/=(const vec2f &r){x/=r.x;y/=r.y;return *this;}
	vec2f &operator()(const float _x,const float _y){x=_x;y=_y;return *this;}
	vec2f &operator-(){x=-x;y=-y;return *this;}
	vec2f operator+(const vec2f &r) const {return vec2f(x+r.x,y+r.y);}
	vec2f operator-(const vec2f &r) const {return vec2f(x-r.x,y-r.y);}
	vec2f operator*(const vec2f &r) const {return vec2f(x*r.x,y*r.y);}
	vec2f operator/(const vec2f &r) const {return vec2f(x/r.x,y/r.y);}
	vec2f operator*(const float v) const {return vec2f(x*v,y*v);}
	vec2f operator/(const float v) const {return vec2f(x/v,y/v);}
	
	float cross(const vec2f &p) const { return x * p.y - y * p.x; }
	float dot(const vec2f &p) const { return x * p.x + y * p.y; }
	float distance(const vec2f &p) const { vec2f t = p - *this; return sqrt(t.x * t.x + t.y * t.y); }
	float length() const { return sqrt(x * x + y * y); }
	float angle(const vec2f &p) { return acosf(this->dot(p)); }
	vec2f &normalize() { return *this /= length(); }
};

class vec3f {
	public:
		GE_ALIGN_BEGIN(16) union{
			struct{
				float x,y,z;
			};
			float data[3];
		}GE_ALIGN_END(16);
		vec3f(const float _x,const float _y,const float _z):x(_x),y(_y),z(_z){}
		vec3f(const float _v){x=y=z=_v;}
		vec3f(const vec2f &xy,const float _z){x=xy.x;y=xy.y;z=_z;}
		vec3f(const float *p){x=*p;y=*(p+1);z=*(p+2);}
		vec3f(){}
		~vec3f(){}
		vec3f &operator+=(const vec3f &v){x+=v.x;y+=v.y;z+=v.z;return *this;}
		vec3f &operator-=(const vec3f &v){x-=v.x;y-=v.y;z-=v.z;return *this;}
		vec3f &operator*=(const float v){x*=v;y*=v;z*=v;return *this;}
		vec3f &operator*=(const vec3f &r){x*=r.x;y*=r.y;z*=r.z;return *this;}
		vec3f &operator/=(const float v){x/=v;y/=v;z/=v;return *this;}
		vec3f &operator/=(const vec3f &r){x/=r.x;y/=r.y;z/=r.z;return *this;}
		vec3f &operator()(const float _x,const float _y,const float _z){x=_x;y=_y;z=_z;return *this;}
		vec3f &operator()(const vec2f &xy,const float _z){x=xy.x;y=xy.y;z=_z;return *this;}
		vec3f &operator-(){x=-x;y=-y;z=-z;return *this;}
		vec3f operator+(const vec3f &r) const {return vec3f(x+r.x,y+r.y,z+r.z);}
		vec3f operator-(const vec3f &r) const {return vec3f(x-r.x,y-r.y,z-r.z);}
		vec3f operator*(const vec3f &r) const {return vec3f(x*r.x,y*r.y,z*r.z);}
		vec3f operator/(const vec3f &r) const {return vec3f(x/r.x,y/r.y,z/r.z);}
		vec3f operator*(const float v) const {return vec3f(x*v,y*v,z*v);}
		vec3f operator/(const float v) const {return vec3f(x/v,y/v,z/v);}
};

class vec4f {
	public:
		GE_ALIGN_BEGIN(16) union{
			struct{
				float x,y,z,w;
			};
			float data[4];
		}GE_ALIGN_END(16);
		vec4f(const float _x,const float _y,const float _z,const float _w):x(_x),y(_y),z(_z),w(_w){}
		vec4f(const float _v){x=y=z=w=_v;}
		vec4f(const vec2f &v1,const vec2f &v2){x=v1.x;y=v1.y;z=v2.x;w=v2.y;}
		vec4f(const vec2f &xy,const float _z,const float _w){x=xy.x;y=xy.y;z=_z;w=_w;}
		vec4f(const vec3f &xyz,const float _w){x=xyz.x;y=xyz.y;z=xyz.z;w=_w;}
		vec4f(const float *p){x=*p;y=*(p+1);z=*(p+2);w=*(p+3);}
		vec4f(){}
		~vec4f(){}
		vec4f &operator+=(const vec4f &v){x+=v.x;y+=v.y;z+=v.z;w+=v.w;return *this;}
		vec4f &operator-=(const vec4f &v){x-=v.x;y-=v.y;z-=v.z;w-=v.w;return *this;}
		vec4f &operator*=(const float v){x*=v;y*=v;z*=v;w*=v;return *this;}
		vec4f &operator*=(const vec4f &r){x*=r.x;y*=r.y;z*=r.z;w*=r.w;return *this;}
		vec4f &operator/=(const float v){x/=v;y/=v;z/=v;w/=v;return *this;}
		vec4f &operator/=(const vec4f &r){x/=r.x;y/=r.y;z/=r.z;w/=r.w;return *this;}
		vec4f &operator()(const float _x,const float _y,const float _z,const float _w){x=_x;y=_y;z=_z;w=_w;return *this;}
		vec4f &operator()(const vec2f &v1,const vec2f &v2){x=v1.x;y=v1.y;z=v2.x;w=v2.y;return *this;}
		vec4f &operator()(const vec2f &xy,const float _z,const float _w){x=xy.x;y=xy.y;z=_z;w=_w;return *this;}
		vec4f &operator()(const vec3f &xyz,const float _w){x=xyz.x;y=xyz.y;z=xyz.z;w=_w;return *this;}
		vec4f &operator-(){x=-x;y=-y;z=-z;w=-w;return *this;}
		vec4f operator+(const vec4f &r) const {return vec4f(x+r.x,y+r.y,z+r.z,w+r.w);}
		vec4f operator-(const vec4f &r) const {return vec4f(x-r.x,y-r.y,z-r.z,w-r.w);}
		vec4f operator*(const vec4f &r) const {return vec4f(x*r.x,y*r.y,z*r.z,w*r.w);}
		vec4f operator/(const vec4f &r) const {return vec4f(x/r.x,y/r.y,z/r.z,w/r.w);}
		vec4f operator*(const float v) const {return vec4f(x*v,y*v,z*v,w*v);}
		vec4f operator/(const float v) const {return vec4f(x/v,y/v,z/v,w/v);}
		void print(){
			std::cout<<"("<<x<<","<<y<<","<<z<<","<<w<<")";
		}
};

// SSE optimized vector
class vec {
	__m128 xmm;
public:
	// constructors/destructors
	vec(){}
	vec(const vec &copy){xmm=copy.xmm;}
	vec(const float &x,const float &y,const float &z,const float &w=0.f){xmm=_mm_setr_ps(x,y,z,w);}
	vec(const float &a){xmm=_mm_set1_ps(a);}
	vec(const float *v){xmm=_mm_loadu_ps(v);}
	vec(const vec3f &v){xmm=_mm_setr_ps(v.x,v.y,v.z,0.f);}
	vec(const vec4f &v){xmm=_mm_load_ps(v.data);}
	vec(const __m128 &_xmm){xmm=_xmm;}
	~vec(){}

	// operators
	vec &operator=(const vec &copy){xmm=copy.xmm; return *this;}
	vec &operator=(const float &a){xmm=_mm_set1_ps(a); return *this;}
	vec &operator=(const float *v){xmm=_mm_loadu_ps(v); return *this;}
	vec &operator=(const vec3f &v){xmm=_mm_setr_ps(v.x,v.y,v.z,0.f); return *this;}
	vec &operator=(const vec4f &v){xmm=_mm_load_ps(v.data); return *this;}
	vec &operator+=(const vec &r){xmm=_mm_add_ps(xmm,r.xmm); return *this;}
	vec &operator-=(const vec &r){xmm=_mm_sub_ps(xmm,r.xmm); return *this;}
	vec &operator*=(const float &r){xmm=_mm_mul_ps(vec(r).xmm,xmm); return *this;}
	vec &operator/=(const float &r){xmm=_mm_div_ps(vec(r).xmm,xmm); return *this;}
	vec &operator*=(const vec &r){xmm=_mm_mul_ps(r.xmm,xmm); return *this;}
	vec &operator/=(const vec &r){xmm=_mm_div_ps(r.xmm,xmm); return *this;}
	vec &operator()(const float &x,const float &y,const float &z,const float &w=0.f){xmm=_mm_setr_ps(x,y,z,w); return *this;}
	vec &operator()(const float &r){xmm=_mm_set1_ps(r); return *this;}
	vec operator-() const { static const __m128 mask = _mm_castsi128_ps(_mm_set1_epi32(0x80000000)); return _mm_xor_ps(xmm, mask); }
	vec operator+(const vec &r)const{return vec(_mm_add_ps(xmm,r.xmm));}
	vec operator-(const vec &r)const{return vec(_mm_sub_ps(xmm,r.xmm));}
	vec operator*(const vec &r)const{return vec(_mm_mul_ps(xmm,r.xmm));}
	vec operator/(const vec &r)const{return vec(_mm_div_ps(xmm,r.xmm));}
	vec operator*(const float r)const{return vec(_mm_mul_ps(vec(r).xmm,xmm));}
	vec operator/(const float r)const{return vec(_mm_div_ps(vec(r).xmm,xmm));}
	vec operator^(const vec &v)const{return _mm_sub_ps(_mm_mul_ps(_mm_shuffle_ps(xmm,xmm,_MM_SHUFFLE(3,0,2,1)),_mm_shuffle_ps(v.xmm,v.xmm,_MM_SHUFFLE(3,1,0,2))),_mm_mul_ps(_mm_shuffle_ps(xmm,xmm,_MM_SHUFFLE(3,1,0,2)),_mm_shuffle_ps(v.xmm,v.xmm,_MM_SHUFFLE(3,0,2,1))));}
	operator vec3f()const{vec4f tr; _mm_store_ps(tr.data,xmm); return vec3f(tr.x,tr.y,tr.z);}
	operator vec4f()const{vec4f r; _mm_store_ps(r.data,xmm); return r;}
	operator __m128()const{return xmm;}

	// fxs
	vec &shuffle(const int &x, const int &y, const int &z, const int w) { xmm = _mm_shuffle_ps(xmm, xmm, _MM_SHUFFLE(x, y, z, w)); return *this; }
	vec &negate() { return *this = -*this; }
	vec &zero() { xmm = _mm_setzero_ps(); return *this; }
	vec &setLength(const float &len) { xmm = _mm_mul_ps(_mm_set1_ps(len), _mm_div_ps(xmm, _mm_sqrt_ps(lengthSq()))); return *this; }
	vec &setDirectionFrom(const vec &v) { __m128 len = _mm_sqrt_ps(lengthSq()); xmm = _mm_mul_ps(len, _mm_div_ps(v.xmm, _mm_sqrt_ps(v.lengthSq()))); return *this; }
	vec &normalize() { xmm = _mm_div_ps(xmm, _mm_sqrt_ps(lengthSq())); return *this; } // precise version, for fast version use mulps and rsqrt (rsqrt uses internal CPU lookup table to compute result)
	vec reflection(const vec &normal) const {float x = 2.f * this->dot(normal); return (*this) - normal * x; }
	float angle(const vec &v) const { return acosf(this->dot(v)); } // angle between vectors in radians (vectors must be normalized)
	float length() const { return _mm_cvtss_f32(xmmLength()); }
	vec lengthSq() const { __m128 m = _mm_mul_ps(xmm, xmm); __m128 t1 = _mm_hadd_ps(m, m); return _mm_hadd_ps(t1, t1);}
	float distance(const vec &v) const { return (*this - v).length(); }
	float dot(const vec &v) const { return _mm_cvtss_f32(xmmDot(v)); }
	vec cross(const vec &v) const { return *this ^ v; }

	// xmm stuff 4U
	vec xmmLength() const { return _mm_sqrt_ps(lengthSq()); }
	vec xmmDistance(const vec &v) const {return (*this - v).xmmLength(); }
	vec xmmVec3() const { __m128 t = _mm_shuffle_ps(xmm, xmm, _MM_SHUFFLE(0, 1, 2, 3)); t = _mm_sub_ss(t, t); return _mm_shuffle_ps(t, t, _MM_SHUFFLE(0, 1, 2, 3)); } // zeroes w component
	vec xmmVec3W() const { return _mm_shuffle_ps(xmm, xmm, _MM_SHUFFLE(0, 1, 2, 3)); }
	vec xmmDot(const vec &v) const { __m128 t = _mm_mul_ps(xmm, v.xmm); __m128 t2 = _mm_hadd_ps(t, t); return _mm_hadd_ps(t2, t2); }
};

class line2d {
public:
	vec2f a, b;

	// const/dest
	line2d(){}
	~line2d(){}
	line2d(const vec2f &_a, const vec2f &_b) : a(_a), b(_b){}
	line2d(float ax, float ay, float bx, float by) : a(ax, ay), b(bx, by){}

	// operators
	line2d operator+(const line2d &r){ return line2d(r.a + a, r.b + b); }
	line2d &operator+=(const line2d &r){ a += r.a; b += r.b; return *this; }
	line2d operator-(const line2d &r){ return line2d(r.a - a, r.b - b); }
	line2d &operator-=(const line2d &r){ a -= r.a; b -= r.b; return *this; }
	line2d operator+(const vec2f &p){ return line2d(p + a, p + b); }
	line2d &operator+=(const vec2f &p){ a += p; b += p; return *this; }
	line2d operator-(const vec2f &p){ return line2d(p - a, p - b); }
	line2d &operator-=(const vec2f &p){ a -= p; b -= p; return *this; }
	line2d operator*(float s){ return line2d(a * s, b * s); }
	line2d &operator*=(float s){ a *= s; b *= s; return *this; }
	line2d operator/(float s){ return line2d(a / s, b / s); }
	line2d &operator/=(float s){ a /= s; b /= s; return *this; }
	line2d &operator()(const vec2f &_a, const vec2f &_b){ a = _a; b = _b; return *this; }
	line2d &operator()(float ax, float ay, float bx, float by){ a(ax, ay); b(bx, by); return *this; }

	// fxs
	float pointOrientation(const vec2f &p) const { return (b - a).cross(p - a); } //!< r > 0 -> left, r > 0 -> right, r == 0 point on line, r -> length
	bool pointOnLine(const vec2f &p) const { return equal(0.f, pointOrientation(p)) && (p.distance(b - a) <= (b - a).length()); }
	bool isParallel(const line2d &l) const { return equal(0.f, (b - a).dot(l.getVector())); }
	bool isPerpendicular(const line2d &l) const { return equal(0.f, (b - a).cross(l.getVector())); }
	float length() const { return a.distance(b); }
	float angle(const line2d &l) const { return getVector().normalize().angle(l.getVector()); }

	// get/set
	vec2f getNormal() const { return vec2f(b.y - a.y, -(b.x - a.x)); }
	vec2f getVector() const { return b - a; }
	vec2f getStart() const { return a; }
	vec2f getEnd() const { return b; }
	line2d &normalize() { b = a + (b - a).normalize(); return *this; }
	line2d &setLength(float l) { b = a + (b - a).normalize() * l; return *this; }
};

class circle2d {
public:
	vec2f o;
	float r;

	// const / dest
	circle2d(){}
	circle2d(const vec2f &mid, float radius) : o(mid), r(radius) {}
	circle2d(float x, float y, float radius) : o(x, y), r(radius) {}
	~circle2d(){}

	// operators
	circle2d &operator+=(const vec2f &p) { o += p; return *this; }
	circle2d operator+(const vec2f &p) const { return circle2d(o + p, r); }
	circle2d &operator()(const vec2f &mid, float radius) { o = mid; r = radius; return *this; }
	circle2d &operator()(float x, float y, float radius) { o(x, y); r = radius; return *this; }

	// fxs
	bool isPointInside(const vec2f &p) const { return p.distance(o) < r; }
	bool intersects(const circle2d &c) const { return o.distance(c.o) < (r + c.r); }
	float getArea() const { return r * r * GE_PI; }
	float getPerimeter() const { return r * GE_2PI; }
	vec2f getPointOnEdge(float rad) const { return vec2f(r * cosf(rad) + o.y, r * sinf(rad) + o.y); }
};

class triangle2d {
public:
	vec2f a, b, c;

	// const / dest
	triangle2d(){}
	triangle2d(const vec2f &_a, const vec2f &_b, const vec2f &_c) : a(_a), b(_b), c(_c){}
	~triangle2d(){}
	
	// opearators
	triangle2d &operator()(const vec2f &_a, const vec2f &_b, const vec2f &_c) { a = _a; b = _b; c = _c; return *this; }
	triangle2d &operator+=(const vec2f &t) { a += t; b += t; c += t; return *this; }
	triangle2d operator+(const vec2f &t) const { return triangle2d(a + t, b + t, c + t); }

	// fxs
	triangle2d &scale(const vec2f &p, float s) { return *this; }
	vec2f getMiddle() const { return vec2f(a.x + b.x + c.x, a.y + b.y + c.y) / 3.f; }
	float getArea() const { return abs((c - a).cross(b - a) * 5.f); }
	float getPerimeter() const { return (b - a).length() + (c - a).length() + (c - b).length(); }
	circle2d getInscribedCircle() const;
	circle2d getCircumscribedCircle() const;
	bool isPointInside(const vec2f &p) const;
	bool intersects(const triangle2d &t) const;
};

class rect2d {
public:
	float left, right, top, bottom;

	// const / dest
	rect2d(){}
	rect2d(float l, float r, float t, float b) : left(l), right(r), top(t), bottom(b) {}
	rect2d(const vec2f &lt, const vec2f &rb) : left(lt.x), right(rb.x), top(lt.y), bottom(rb.y) {}
	~rect2d(){}

	// operators
	rect2d &operator()(float l, float r, float t, float b) { left = l; right = r; top = t; bottom = b; return *this; }
	rect2d &operator()(const vec2f &lt, const vec2f &rb) { left = lt.x; right = rb.x; top = lt.y; bottom = rb.y; return *this; }
	rect2d &operator+=(const vec2f &t) { left += t.x; right += t.x; top += t.y; bottom += t.y; return *this; }
	rect2d operator+(const vec2f &t) const { return rect2d(left + t.x, right + t.x, top + t.y, bottom + t.y); }

	// fxs
	float getArea() const { return getWidth() * getHeight(); }
	float getPerimeter() const { return 2.f * getWidth() + 2.f * getHeight(); }
	float getDiagonal() const { return sqrtf(getWidth() * getWidth() + getHeight() * getHeight()); }
	float getWidth() const { return abs(right - left); }
	float getHeight() const { return abs(top - bottom); }
	vec2f getCenter() const { return vec2f((left + right) / 2.f, (top + bottom) / 2.f); }
	vec2f getDimmensions() const { return vec2f(getWidth(), getHeight()); }
	void repair() { if(left > right) std::swap(left, right); if(bottom > top) std::swap(bottom, top); }
	bool isPointInside(const vec2f &p) const { return p.x <= right && p.x >= left && p.y >= bottom && p.y <= top; }
	bool intersects(const rect2d &r) const { return right > r.left && left < r.right && bottom < r.top && top > r.bottom; }
	bool includes(const rect2d &r) const { return r.right < right && r.left > left && r.top < top && r.bottom > bottom; }
	rect2d getUnion(const rect2d &r) const { return rect2d(std::max(left, r.left), std::min(right, r.right), std::max(top, r.top), std::min(bottom, r.bottom)); }
};

class polygon2d {
public:
	std::vector<vec2f> points;

	// const / dest
	polygon2d(){}
	~polygon2d(){}

	// operators
	polygon2d &operator+=(const vec2f &t);
	polygon2d operator+(const vec2f &t) const;
	
	//fxs
	float getArea() const;
	float getPerimeter() const;
	vec2f getCenter() const;
	float getMaxx() const;
	float getMaxy() const;
	float getMinx() const;
	float getMiny() const;
	rect2d getBoundingRect() const;
	void deleteDuplicates();
	bool isPointInside(const vec2f &p) const;
};

class quad2d {
public:
	vec2f a, b, c, d;

	// const / dest
	quad2d(){}
	quad2d(const vec2f &_a, const vec2f &_b, const vec2f &_c, const vec2f &_d) : a(_a), b(_b), c(_c), d(_d) {}
	quad2d(float _l, float _r, float _t, float _b) : a(_l, _t), b(_l, _b), c(_r, _b), d(_r, _t) {}
	~quad2d(){}

	// operators
	quad2d &operator()(const vec2f &_a, const vec2f &_b, const vec2f &_c, const vec2f &_d) { a = _a; b = _b; c = _c; d = _d; return *this; }
	quad2d &operator()(float _l, float _r, float _t, float _b) { a(_l, _t); b(_l, _b); c(_r, _b); d(_r, _t); return *this; }
	quad2d &operator+=(const vec2f &t) { a += t; b += t; c += t; d += t; return *this; }
	quad2d operator+(const vec2f &t) { return quad2d(a + t, b + t, c + t, d + t); }

	// fxs
	float getArea() const;
	float getPerimeter() const;
	vec2f getCenter() const;
	rect2d getBoundingRect() const;
	bool isPointInside(const vec2f &p) const;
};

class line {
public:
	vec a, b;

	line(){}
	line(const vec &_a, const vec &_b) : a(_a), b(_b) {}
	line(float ax, float ay, float az, float bx, float by, float bz) : a(ax, ay, az), b(bx, by, bz) {}
	~line(){}

	line &operator()(const vec &_a, const vec &_b) { a = _a; b = _b; return *this; }
	line &operator()(float ax, float ay, float az, float bx, float by, float bz) { a(ax, ay, az); b(bx, by, bz); return *this; }
	
	vec closestPointClamp(const vec &v) const;
	vec closestPoint(const vec &v) const;
	float distance(const vec &p) const;
	float length() const;
};

class triangle {
public:
	vec a, b, c;

	triangle(){}
	triangle(const vec &_a, const vec &_b, const vec &_c) : a(_a), b(_b), c(_c) {}
	~triangle(){}

	triangle &operator()(const vec &_a, const vec &_b, const vec &_c) { a = _a; b = _b; c = _c; return *this; }
};

class sphere {
public:
	vec cr; // xyz - center, w - radius

	sphere(){}
	sphere(const vec &_cr) : cr(_cr) {}
	sphere(const vec &center, float radius) { (*this)(center, radius); }
	~sphere(){}

	sphere &operator()(const vec &_cr) { cr = _cr; return *this; }
	sphere &operator()(const vec &center, float radius) { cr = _mm_set_ss(radius); cr = _mm_shuffle_ps(cr, cr, _MM_SHUFFLE(0, 1, 1, 1)); cr = _mm_add_ps(cr, center); return *this; }

	vec getCenter() const { return cr.xmmVec3(); }
	float getRadius() const { return _mm_cvtss_f32(cr.xmmVec3W()); }
	bool isValid() const { __m128 r = _mm_shuffle_ps(cr, cr, _MM_SHUFFLE(3, 3, 3, 3)); r = _mm_cmpge_ss(r, _mm_setzero_ps()); return 0 != _mm_cvtss_si32(r); }
	bool contains(const sphere &s) const;
	bool contains(const vec &p) const;
	bool intersects(const sphere &s) const;
	
	sphere &zero() { cr = _mm_setzero_ps(); return *this; }
	sphere &expand(const sphere &s);
	sphere &expand(const vec &p);
};

class aabbox {
public:
	vec pmin, pmax;

	aabbox(){}
	aabbox(const vec &pmi, const vec &pma) : pmin(pmi), pmax(pma) {}
	aabbox(const sphere &sp) { this->operator()(sp); }
	~aabbox(){}

	aabbox &operator()(const vec &pmi, const vec &pma) { pmin = pmi; pmax = pma; return *this; }
	aabbox &operator()(const sphere &sp) { __m128 r = _mm_shuffle_ps(sp.cr, sp.cr, _MM_SHUFFLE(3, 3, 3, 3)); pmin = sp.cr - r; pmax = sp.cr + r; return *this; }
	
	vec getCenter() const { return (pmax + pmin) / 2.f; }
	void getEdges(vec *o) const; // gets 8 edges
	float getDiagonal() const { return pmin.distance(pmax); }
	vec getDimmensions() const { return pmax - pmin; }
	bool contains(const vec &p) const;
	bool contains(const sphere &s) const { return contains(aabbox(s)); }
	bool contains(const aabbox &box) const;
	bool isValid() const;

	aabbox &zero() { pmin = pmax = 0.f; return *this; }
	aabbox &extend(const vec &p) { pmin = _mm_min_ps(pmin, p); pmax = _mm_max_ps(pmax, p); return *this; }
	aabbox &extend(const aabbox &box) { pmin = _mm_min_ps(pmin, box.pmin); pmax = _mm_max_ps(pmax, box.pmax); return *this; }
	aabbox &extend(const sphere &sp) { return extend(aabbox(sp)); }
	aabbox &repair() { __m128 t = pmin; pmin = _mm_min_ps(pmin, pmax); pmax = _mm_max_ps(t, pmax); return *this; }
};

class obbox {
	vec dotAll(const vec &p) const;
public:
	vec center, scale, axis[3];

	obbox(){}
	obbox(const vec &c, const vec &s, const vec *ax) { (*this)(c, s, ax); }
	obbox(const vec &c, const vec &s, const vec &axx, const vec &axy, const vec &axz) { (*this)(c, s, axx, axy, axz); }
	obbox(const aabbox &box) { (*this)(box); }
	obbox(const sphere &sp) { (*this)(sp); }
	~obbox(){}

	obbox &operator()(const vec &c, const vec &s, const vec *ax) { center = c; axis[0] = ax[0]; axis[1] = ax[1]; axis[2] = ax[2]; scale = s; return *this; }
	obbox &operator()(const vec &c, const vec &s, const vec &axx, const vec &axy, const vec &axz) { center = c; axis[0] = axx; axis[1] = axy; axis[2] = axz; scale = s; return *this; }
	obbox &operator()(const aabbox &box) { center = box.getCenter(); scale = box.getDimmensions() * .5f; axis[0] = vec4f(1.f, 0.f, 0.f); axis[1] = vec4f(0.f, 1.f, 0.f); axis[2] = vec4f(0.f, 0.f, 1.f); return *this; }
	obbox &operator()(const sphere &sp) { center = sp.getCenter(); scale = sp.getRadius(); axis[0] = vec4f(1.f, 0.f, 0.f); axis[1] = vec4f(0.f, 1.f, 0.f); axis[2] = vec4f(0.f, 0.f, 1.f); return *this; }

	void getEdges(vec *o) const;
	float diagonal() const;
	bool contains(const vec &p) const;
	vec closestPoint(const vec &p) const;
	vec minPointAlongNormal(const vec &normal) const;
	vec maxPointAlongNormal(const vec &normal) const;

	obbox &zero() { center = scale = axis[0] = axis[1] = axis[2] = _mm_setzero_ps(); return *this; }
	obbox &normalize() { for(int i = 0; i < 3; ++i) axis[i].normalize(); return *this; }
};

}}

//~

