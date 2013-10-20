#include "math.h"
#include <numeric>

namespace granite{
namespace base{

namespace {
bool rayUpIntersectsLine(const vec2f &p, const vec2f &a, const vec2f &b){
	float intersect_y;
	
	if(p.y <= std::max(a.y, b.y)){
		if((p.x >= std::min(a.x, b.x)) && (p.x <= std::max(a.x, b.x))){
			if(p.y <= std::min(a.y, b.y))
				return true;
			intersect_y = a.y + ((p.y - a.y) * (b.y - a.y)) / (b.x - a.x);
			if(intersect_y >= p.y)
				return true;
		}
	}
	return false;
}
}

circle2d triangle2d::getInscribedCircle() const {
	circle2d ret;
	
	// calculate radius
	float area2 = getArea() * 2.f;
	float perp = getPerimeter();
	ret.r = area2 / perp;

	// calc midpoint
	ret.o = c * (b - a).length();
	ret.o += a * (c - b).length();
	ret.o += b * (a - c).length();
	ret.o /= perp;
	
	return ret;
}

circle2d triangle2d::getCircumscribedCircle() const {
	// based on nv_algebra
	circle2d ret;
	
	vec2f e0, e1;
	float d1, d2, d3;
	float c1, c2, c3, oo_c;

	e0 = c - a;
	e1 = b - a;
	d1 = e0.dot(e1);
	      
	e0 = c - b;
	e1 = a - b;
	d2 = e0.dot(e1);
	      
	e0 = a - c;
	e1 = b - c;
	d3 = e0.dot(e1);
	      
	c1 = d2 * d3;
	c2 = d3 * d1;
	c3 = d1 * d2;
	oo_c = 1.f / (c1 + c2 + c3);

	ret.o = a * (c2 + c3);
	ret.o += b * (c3 + c1);
	ret.o += c * (c1 + c2);
	ret.o *= oo_c * 0.5f;

	ret.r = 0.5f * sqrtf((d1 + d2) * (d2 + d3) * (d3 + d1) * oo_c);
	return ret;
}

bool triangle2d::isPointInside(const vec2f &p) const{
	int intersections = 0;
	
	if(rayUpIntersectsLine(p, a, b))
		++intersections;
	if(rayUpIntersectsLine(p, c, b))
		++intersections;
	if(rayUpIntersectsLine(p, a, c))
		++intersections;

	return 0 != intersections % 2;
}

bool triangle2d::intersects(const triangle2d &t) const{
	return isPointInside(t.a)
		|| isPointInside(t.b)
		|| isPointInside(t.c)
		|| t.isPointInside(a)
		|| t.isPointInside(b)
		|| t.isPointInside(c);
}

polygon2d &polygon2d::operator+=(const vec2f &t) {
	for(auto &p : points)
		p += t;
	return *this;
}

polygon2d polygon2d::operator+(const vec2f &t) const {
	polygon2d ret = *this;
	return ret += t;
}
	
float polygon2d::getArea() const {
	if(points.size() < 3)
		return 0.f;

	float area = 0.f;
	auto ia = points.cbegin(), ib = ia + 1, ic = ia + 2;

	for(; ic != points.cend(); ++ic, ++ib){
		triangle2d tri(*ia, *ib, *ic);
		area += tri.getArea();
	}

	return area;
}

float polygon2d::getPerimeter() const {
	if(points.size() < 2)
		return 0.f;

	float per = 0.f;
	auto prv = points.cbegin();
	
	for(auto curr = points.cbegin() + 1; curr != points.cend(); ++curr){
		per += prv->distance(*curr);
		prv = curr;
	}
	per += points.front().distance(points.back());
	
	return per;
}

vec2f polygon2d::getCenter() const {
	return std::accumulate(points.cbegin(), points.cend(), vec2f(0.f, 0.f)) / float(points.size());
}

float polygon2d::getMaxx() const {
	return std::max_element(points.cbegin(), points.cend(), [](const vec2f &a, const vec2f &b) -> bool { return a.x > b.x; })->x;
}

float polygon2d::getMaxy() const {
	return std::max_element(points.cbegin(), points.cend(), [](const vec2f &a, const vec2f &b) -> bool { return a.y > b.y; })->y;
}

float polygon2d::getMinx() const {
	return std::min_element(points.cbegin(), points.cend(), [](const vec2f &a, const vec2f &b) -> bool { return a.x < b.x; })->x;
}

float polygon2d::getMiny() const {
	return std::max_element(points.cbegin(), points.cend(), [](const vec2f &a, const vec2f &b) -> bool { return a.y < b.y; })->y;
}

rect2d polygon2d::getBoundingRect() const {
	auto mmx = std::minmax_element(points.cbegin(), points.cend(), [](const vec2f &a, const vec2f &b) -> bool { return a.x < b.x; });
	auto mmy = std::minmax_element(points.cbegin(), points.cend(), [](const vec2f &a, const vec2f &b) -> bool { return a.y < b.y; });
	return rect2d(mmx.first->x, mmx.second->x, mmy.first->y, mmy.second->y);
}

void polygon2d::deleteDuplicates() {
	std::unique(points.begin(), points.end(), [](const vec2f &a, const vec2f &b) -> bool { return equal(a.x, b.x) && equal(a.y, b.y); });
}

bool polygon2d::isPointInside(const vec2f &p) const {
	if(points.size() < 2)
		return equal(points.front().x, p.x) && equal(points.front().y, p.y);

	int intersections = 0;
	auto prv = points.cbegin(), curr = prv + 1;
	
	for(; curr != points.cend(); ++curr){
		if(rayUpIntersectsLine(p, *curr, *prv))
			++intersections;
		prv = curr;
	}

	if(rayUpIntersectsLine(p, points.front(), points.back()))
		++intersections;

	return 0 != intersections % 2;
}

float quad2d::getArea() const {
	return triangle2d(a, b, c).getArea() + triangle2d(a, c, d).getArea();
}

float quad2d::getPerimeter() const {
	return a.distance(b) + b.distance(c) + c.distance(d) + d.distance(a);
}

vec2f quad2d::getCenter() const {
	return (a + b + c + d) / 4.f;
}

rect2d quad2d::getBoundingRect() const {
	return rect2d(std::min({a.x, b.x, c.x, d.x}), std::max({a.x, b.x, c.x, d.x}), std::min({a.y, b.y, c.y, d.y}), std::max({a.y, b.y, c.y, d.y}));
	
}

bool quad2d::isPointInside(const vec2f &p) const {
	int intersections = 0;
	if(rayUpIntersectsLine(p, a, b))
		++intersections;
	if(rayUpIntersectsLine(p, b, c))
		++intersections;
	if(rayUpIntersectsLine(p, c, d))
		++intersections;
	if(rayUpIntersectsLine(p, d, a))
		++intersections;
	return 0 != intersections % 2;
}

vec line::closestPointClamp(const vec &v) const {
	vec ab = b - a;
	float t = vec(v - a).dot(ab) / ab.dot(ab); // projekcja d(t) = a + t * (b - a)
	t = clip(0.f, t, 1.f); // przyciecie do granic odcinka
	return a + ab * t;
}

vec line::closestPoint(const vec &v) const {
	vec ab = b - a;
	float t = vec(v - a).dot(ab) / ab.dot(ab);
	return a + ab * t;
}

float line::distance(const vec &p) const {
	return closestPoint(p).distance(p);
}

float line::length() const {
	return a.distance(b);
}

sphere &sphere::expand(const sphere &s) {
	vec c1 = getCenter(), c2 = s.getCenter();
	vec r = _mm_add_ss(_mm_add_ss(c1.xmmDistance(c2), s.cr.xmmVec3W()), cr.xmmVec3W()); // distance + r1 + r2
	r = _mm_mul_ss(r, _mm_set_ss(.5f));
	cr = (c1 + c2) * .5f; // new center
	r = _mm_shuffle_ps(cr, r, SSE_RSHUFFLE(0, 2, 3, 3)); // write radius
	cr = _mm_shuffle_ps(cr, r, SSE_RSHUFFLE(0, 1, 1, 3));
	return *this;
}

sphere &sphere::expand(const vec &p) {
	// r = max(distance(p, center), r)
	__m128 distance = p.xmmDistance(getCenter());
	__m128 t = _mm_max_ss(distance, cr.xmmVec3W()); // compute new radius
	t = _mm_shuffle_ps(cr, t, SSE_RSHUFFLE(0, 2, 3, 3)); // t = (cr.x, cr.z, t.w, t.w)
	cr = _mm_shuffle_ps(cr, t, SSE_RSHUFFLE(0, 1, 1, 3)); // cr = (cr.x, cr.y, t.y, t.w)
	return *this;
}

bool sphere::contains(const sphere &s) const {
	// distance + s.r < r => distance < r - s.r
	__m128 rsub = _mm_sub_ps(cr.xmmVec3W(), s.cr.xmmVec3W());
	__m128 distance = getCenter().xmmDistance(s.getCenter());
	return 0 != _mm_cvtss_si32(_mm_cmple_ss(distance, rsub));
}

bool sphere::contains(const vec &p) const {
	vec distance = p.xmmDistance(getCenter());
	__m128 t = _mm_cmple_ss(distance, cr.xmmVec3W()); // distance <= r
	return 0 != _mm_cvtss_si32(t);
}

bool sphere::intersects(const sphere &s) const {
	__m128 rsum = _mm_add_ss(s.cr.xmmVec3W(), cr.xmmVec3W()); // r1 + r2
	__m128 distance = s.getCenter().xmmDistance(getCenter()); // center distance
	return 0 != _mm_cvtss_si32(_mm_cmple_ss(distance, rsum)); // distance < r1 + r2
}

void aabbox::getEdges(vec *o) const {
	const vec &a = pmin;
	const vec &b = pmax;
	o[0] = b;
	o[1] = _mm_shuffle_ps(b, a, SSE_RSHUFFLE(0, 2, 1, 3)); // (b.x, b.z, a.y)
	o[1] = _mm_shuffle_ps(o[1], b, SSE_RSHUFFLE(0, 2, 2, 3)); // (b.x, a.y, b.z)
	o[2] = _mm_shuffle_ps(a, b, SSE_RSHUFFLE(0, 1, 2, 3)); // (a.x, a.y, b.z)
	o[3] = _mm_shuffle_ps(b, a, SSE_RSHUFFLE(2, 1, 0, 3)); // (b.z, b.y, a.x)
	o[3] = _mm_shuffle_ps(o[3], b, SSE_RSHUFFLE(2, 1, 2, 3)); // (a.x, b.y, b.z)
	o[4] = _mm_shuffle_ps(b, a, SSE_RSHUFFLE(0, 1, 2, 3)); // (b.x, b.y, a.z)
	o[5] = _mm_shuffle_ps(a, b, SSE_RSHUFFLE(1, 2, 0, 3)); // (a.y, a.z, b.x)
	o[5] = _mm_shuffle_ps(o[5], a, SSE_RSHUFFLE(2, 0, 2, 3)); // (b.x, a.y, a.z);
	o[6] = a;
	o[7] = _mm_shuffle_ps(a, b, SSE_RSHUFFLE(0, 2, 1, 3)); // (a.x, a.z, b.y)
	o[7] = _mm_shuffle_ps(o[7], a, SSE_RSHUFFLE(0, 2, 2, 3)); // (a.x, b.y, a.z)
}

bool aabbox::contains(const vec &p) const {
	__m128 le = _mm_cmple_ps(pmin, p); // test if p >= min
	__m128 ge = _mm_cmple_ps(p, pmax); // p <= max
	__m128 t = _mm_and_ps(le, ge); // combine results
	t = _mm_shuffle_ps(t, t, _MM_SHUFFLE(2, 2, 1, 0)); // ignore w component
	return 0 != _mm_movemask_epi8(_mm_castps_si128(t)); // cast to int and shift & or register
}

bool aabbox::contains(const aabbox &box) const {
	__m128 le = _mm_cmple_ps(pmin, box.pmin); // box.min >= min
	__m128 ge = _mm_cmpge_ps(pmax, box.pmax); // box.max <= max
	__m128 t = _mm_and_ps(le, ge);
	t = _mm_shuffle_ps(t, t, _MM_SHUFFLE(2, 2, 1, 0));
	return 0 != _mm_movemask_epi8(_mm_castps_si128(t));
}

bool aabbox::isValid() const {
	__m128 test = _mm_cmple_ps(pmin, pmax);
	test = _mm_shuffle_ps(test, test, _MM_SHUFFLE(2, 2, 1, 0));
	return 0 != _mm_movemask_epi8(_mm_castps_si128(test));
}

}}
