#include <base/base.h>

template<typename... P> class delegate
{
	static const int argsCount=sizeof...(P);
	typedef std::function<void(P...)> tdSlot;
	tdSlot m_slot;
public:
	delegate(){}
	~delegate(){}
	void connect(const tdSlot &f){
		m_slot=f;
	}
	template<typename MP,typename TP>void connect(MP mp,TP *tp){
		granite::base::detail::bind<MP,TP,tdSlot>::template get<argsCount>(mp,tp,m_slot);
	}
	void fire(P... args){
		m_slot(args...);
	}
	void operator=(const tdSlot &f){
		return connect(f);
	}
};


class foo{
	public:
		void zzz(int &x){
			std::cout<<"\nfoo::zzz="<<x;
		}
		void zzzf(float x){
			std::cout<<"\nfoo::zzzf="<<x;
			}
		void zzzff(float x,float y){
			std::cout<<"\nfoo::zzzf="<<x<<", "<<y;
		}
};

class bar{
	public:
		void abc(int &x){
			std::cout<<"\nbar::abc="<<x;
		}
		void abcf(float x){
			std::cout<<"\nbar::abcf="<<x;
		}
};

void foobar(int &xx){
	std::cout<<"\nfoobar="<<xx;
	xx=777;
}

int main(int argc, char**argv){
	using namespace granite::base;
	
	foo obiekt;
	bar obiekt2;
	
	signal<int&> sigs;
	sigs+=[](int &a){std::cout<<"\nlambda: "<<a<<" c++11!";};
	sigs.connect(foobar);
	slotId id2=sigs.connect(&foo::zzz,&obiekt);
	slotId id1=sigs.connect(&foo::zzz,&obiekt);
	sigs.connect(&bar::abc,&obiekt2);
	int modMe=667;
	sigs.fire(modMe);
	std::cout<<"\nmodMe after sigslot: "<<modMe<<"\n\n";

	sigs.disconnect(id1);
	sigs.disconnect(id2);
	sigs.fire(modMe);

	signal<float> sigs2;
	sigs2.connect(&foo::zzzf,&obiekt);
	sigs2.connect(&foo::zzzf,&obiekt);
	sigs2.connect(&bar::abcf,&obiekt2);
	float ff=555;
	sigs2.fire(ff);
	
	delegate<float,float> df;
	df.connect(&foo::zzzff,&obiekt);
	df.fire(5,10);
	df.connect(std::bind(&foo::zzzff,&obiekt,std::placeholders::_1,std::placeholders::_2)); // mo¿na te¿ tak
	df.fire(10,15);

	int intArg=66;
	delegate<int&> di;
	di=[](int &o){std::cout<<"\ninside lambda: "<<o; ++o;};
	di.fire(intArg);
	di.fire(intArg);
	std::cout<<"\nchanged value: "<<intArg;

	di.connect(&foo::zzz,&obiekt);
	di.fire(intArg);
	
	return 0;
}
