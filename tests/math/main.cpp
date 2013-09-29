#include <base/base.h>

void printVec(const granite::base::vec4f &v,const std::string &name){
	std::cout<<"\nvector-print: "<<name<<"("<<v.x<<","<<v.y<<","<<v.z<<","<<v.w<<")";
}
void printVec3(const granite::base::vec3f &v,const std::string &name){
	std::cout<<"\nvector-print: "<<name<<"("<<v.x<<","<<v.y<<","<<v.z<<")";
}

int main(int argc, char**argv){
	using namespace granite::base;
	
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
	
	return 0;
}
