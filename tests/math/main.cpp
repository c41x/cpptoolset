#include <base/base.hpp>

using namespace granite::base;

void printVec(const vec4f &v, const std::string &name){
	std::cout << "\nvector-print: " << name << "(" << v.x << "," << v.y << "," << v.z << "," << v.w << ")";
}

void printVec3(const vec3f &v, const std::string &name){
	std::cout << "\nvector-print: " << name << "(" << v.x << "," << v.y << "," << v.z << ")";
}

void printMatrix(const matrix &m, const std::string &desc){
	vec4f x = m.x;
	vec4f y = m.y;
	vec4f z = m.z;
	vec4f t = m.t;
	std::cout << "\n" << desc;
	std::cout << "\n| " << x.x << ", " << x.y << ", " << x.z << ", " << x.w << " |";
	std::cout << "\n| " << y.x << ", " << y.y << ", " << y.z << ", " << y.w << " |";
	std::cout << "\n| " << z.x << ", " << z.y << ", " << z.z << ", " << z.w << " |";
	std::cout << "\n| " << t.x << ", " << t.y << ", " << t.z << ", " << t.w << " |";
}

void printBool(bool test, const std::string &desc) {
	std::cout << "\n" << desc << ": " << test;
}

int main(int argc, char**argv){
	std::cout << std::boolalpha;

	vec t(1.f,2.f,3.f,0.f);
	vec tt(1.f,3.f,3.f,666.f);
	vec mul(2.f);
	vec r=(t+tt)*mul;
	printVec3(t,"t");
	printVec3(tt,"tt");
	printVec(mul,"mul");
	printVec(r,"r=(t+tt)*mul ");
	printVec(-r,"-r");
	printVec(r.zero(),"r.zero() ");
	printVec(r-r,"r-r");

	vec a(2,4,5);
	vec b=vec3f(5,-1,-1);
	printVec(a,"a");
	printVec(b,"b");
	printVec(a^b,"a^b");

	a(3,2,1);
	b=vec4f(1,2,3,0);
	printVec(a,"a");
	printVec(b,"b");
	std::cout<<"\na dot b = "<<a.dot(b);

	a(5,5,0);
	float lenA=a.length();
	std::cout<<"\nlength of a: |(5,5,0)| = "<<lenA;
	a(2,2,0);
	std::cout<<"\nlength of a: |(2,2,2)| = "<<a.length();
	a(0,GE_SQRT2,GE_SQRT2);
	std::cout<<"\nlength of a: |(0,sqrt(2),sqrt(2))| = "<<a.length();
	printVec(a.normalize(),"(0,sqrt(2),sqrt(2)) normalized");
	std::cout<<"\nlength of normalized = "<<a.length();

	vec x(234,666,345,0);
	x.setLength(5.f);
	printVec(x,"out target vector to test setLength function (after setLength): ");
	std::cout<<"\nset (234,666,345) length to 5 = "<<x.length();

	vec la(10,10,10),lb(10-2,10,10-2);
	std::cout<<"\ndistance test, should be 2.82843 ?= "<<la.distance(lb);

	vec aa(0,1,0),ab(1,0,0);
	std::cout<<"\n angle between (1,1,1) an (1,0,0) in degrees : "<<(aa.normalize().angle(ab) * 180.f)/GE_PI;

	vec dir=vec3f(5,0,0);
	printVec(dir.setDirectionFrom(aa),"(5,0,0).setDirectionFrom(0,1,0) = ");

	vec shot(1.f,-1.f,1.f);
	vec normal(0.f,1.f,0.f);
	printVec3(shot.reflection(normal),"(1,-1,1) reflection normal (0,1,0) = ");

	vec sqr(GE_SQRT2,GE_SQRT2,0.f,0.f);
	printVec(sqr.lengthSq(),"lenSq(sqrt(2),sqrt(2),0,0)=");

	vec4f leTest(5,10,15,20);
	vec leTestSSE(leTest);
	printVec(leTestSSE, "little-endiannes test:");

	// test aabbox
	aabbox bbox(vec(1.f, 2.f, 3.f), vec(-1.f, -2.f, -3.f));
	bbox.repair();
	bool ta = bbox.contains(vec(0.f, 0.f, 0.f));
	bool tb = bbox.contains(vec(2.f, 0.f, 0.f));
	std::cout << "\ncontains: a = " << (ta ? "true" : "false") << " contains b = " << (tb ? "true" : "false");

	std::cout << "\nedges: ";
	vec edges[8];
	bbox.getEdges(edges);
	for(vec &e : edges){
		printVec(e, "");
	}

	// test sphere
	sphere sp(vec(0.f, 1.f, 0.1f, 4.f));
	printVec(sp.cr, "sphere data: ");
	printVec(sp.getCenter(), "sphere center: ");
	std::cout << "\nradius: " << sp.getRadius();
	std::cout << "\nis valid? " << sp.isValid();
	std::cout << "\ncontains (0, 0, 0) " << sp.contains(vec(0.f, 0.f, 0.f));
	std::cout << "\ncontains (0, 1, 0) " << sp.contains(vec(0.f, 1.f, 0.f));
	std::cout << "\ncontains (1, 1, 1) " << sp.contains(vec(1.f, 1.f, 1.f));
	std::cout << "\ncontains (0, 5, 0) " << sp.contains(vec(0.f, 5.f, 0.f));
	std::cout << "\ncontains (0, 5.1, 0) " << sp.contains(vec(0.f, 5.1f, 0.f));
	printVec(sp.expand(vec(10.f, 10.f, 10.f)).cr, "expand sphere to contain (10, 10, 10): ");

	// obbox
	obbox obo(vec(0.f, 0.f, 0.f), vec(1.f, 2.f, 3.f), vec(1.f, 0.f, 0.f), vec(0.f, 1.f, 0.f), vec(0.f, 0.f, 1.f));
	std::cout << "\nedges: ";
	obo.getEdges(edges);
	for(vec &e : edges){
		printVec(e, "");
	}
	std::cout << "\ndiagonal = " << obo.diagonal();
	std::cout << "\ncontains (0, 0, 0) " << obo.contains(vec(0.f, 0.f, 0.f));
	std::cout << "\ncontains (0, 1, 0) " << obo.contains(vec(0.f, 1.f, 0.f));
	std::cout << "\ncontains (1, 1, 1) " << obo.contains(vec(1.f, 1.f, 1.f));
	std::cout << "\ncontains (-1, -1, -1) " << obo.contains(vec(-1.f, -1.f, -1.f));
	std::cout << "\ncontains (0, 5, 0) " << obo.contains(vec(0.f, 5.f, 0.f));
	std::cout << "\ncontains (0, -2.01, 0) " << obo.contains(vec(0.f, -2.01f, 0.f));
	std::cout << "\ncontains (0, 5.1, 0) " << obo.contains(vec(0.f, 5.1f, 0.f));

	printVec(obo.closestPoint(vec(0.5f, 100.f, 0.f)), "closest point to (0.5, 100, 0)");
	printVec(obo.minPointAlongNormal(vec(5.f, 10.f, 0.f).normalized()), "min point along normal (5, 10, 0)");
	printVec(obo.maxPointAlongNormal(vec(5.f, 10.f, 0.f).normalized()), "maxpoint along normal (5, 10, 0)");

	// test plane
	plane pl(vec(0.f, 1.f, 0.f), -5.f);
	std::cout << "\ndistance to (0, 0, 0) : " << pl.distance(vec(0.f, 0.f, 0.f));
	std::cout << "\ndistance to (0, 10, 0) : " << pl.distance(vec(0.f, 10.f, 0.f));
	std::cout << "\nis above bbox(-1,-1,-1,1,1,1) : " << pl.isAnyAbove(aabbox(vec(-1.f, -1.f, -1.f), vec(1.f, 1.f, 1.f)));
	std::cout << "\nis above bbox(8,9,9,10,10,10) : " << pl.isAnyAbove(aabbox(vec(8.f, 9.f, 9.f), vec(10.f, 10.f, 10.f)));
	std::cout << "\nis above bbox(8,9,0,10,10,10) : " << pl.isAnyAbove(aabbox(vec(8.f, 9.f, 0.f), vec(10.f, 10.f, 10.f)));

	// matrix testing
	{
		matrix m;
		m.identity();
		printMatrix(m, "set to identity");
		printBool(m.isIdentity(), "is identity");
		printMatrix(m.setTranslation(vec(1.f, 2.f, 3.f)), "set translation to (1, 2, 3)");
		printBool(m.isIdentity(), "is above identity?");
		printMatrix(m.setScale(vec(4.f, 5.f, 6.66f)), "set scale to (4, 5, 6.66)");
		printVec(m * vec(3.f, 7.f, 10.f), "transform vec (3, 7, 10)");
		printVec(m * vec(0.f, 0.f, 0.f), "transform vec (0)");
		printMatrix(m.setScale(vec(2.f, 4.f, 6.f)), "apply new scale (2, 4, 6)");
		matrix tma = m, t2 = m;
		printMatrix(m.inverse(), "inverse above");
		printMatrix(tma.setScale(vec(1.f, 1.f, 1.f)), "set scale to vec(1) to test simple inverse");
		printMatrix(tma.inverseSimple(), "simple inverse above");
		printMatrix(t2 * t2, "mul matrix");
		printMatrix(matrix().setRotation(vec(1.f, 0.f, 0.f), degToRad(180.f)), "setRotation vec(1,0,0), pi");
		printMatrix(matrix().setRotation(vec(1.f, 1.f, 0.f).normalized(), degToRad(90.f)), "setRotation vec(1,1,0), pi/2");
	}

	// shuffle test
	vec aax(0.f, 1.f, 2.f, 3.f);
	std::cout << toStr(aax) << std::endl;
	aax.shuffle<0, 0, 1, 3>();
	std::cout << toStr(aax) << std::endl;

	// simd_vector
	{
		std::cout << std::endl;
		simd_vector<float> v;
		v.resize(10);
		float fi = 0.0f;
		for (auto &e : v)
			e = fi += 1.001f;
		for (auto e : v)
			std::cout << e << std::endl;
		auto e = v.erase(v.begin(), v.begin() + 5);
		v.erase(e);
		v.resize(10, 666.0f);
		std::cout << std::endl;
		for (auto e : v)
			std::cout << e << std::endl;
	}

	// polygon
	{
		std::cout << std::endl;
		polygon2d p;
		p.points.resize(11);
		float fi = 0.0f;
		for (auto &pt : p.points) {
			pt = vec2f(fi, fi + 1.0f);
			fi += 1.1f;
		}

		for (auto &pt : p.points) {
			std::cout << toStr(pt) << std::endl;
		}

		std::cout << "center: " << toStr(p.getCenter()) << std::endl;
	}

	std::cout << "\nfinished\n";
	std::cout << std::flush;

	return 0;
}
