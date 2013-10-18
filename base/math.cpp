#include "math.h"

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

}}
