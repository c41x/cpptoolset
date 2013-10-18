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

}}
