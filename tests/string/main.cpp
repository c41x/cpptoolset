#include <base/base.h>

void PrintStats(std::vector<double> timings) {
    double fastest = std::numeric_limits<double>::max();

    std::cout << std::fixed;// << std::setprecision(2);
    std::cout << "[";
    for (size_t i = 1 ; i<timings.size()-1 ; ++i) {
        fastest = std::min(fastest, timings[i]);
        std::cout << timings[i] << ",";
    }
    std::cout << timings.back();
    std::cout << "]";

    double sum = 0.0;
    for (size_t i = 1 ; i<timings.size() ; ++i) {
        sum += timings[i];
    }
    double avg = sum / static_cast<double>(timings.size()-1);

    sum = 0.0;
    for (size_t i = 1 ; i<timings.size() ; ++i) {
        timings[i] = pow(timings[i]-avg, 2);
        sum += timings[i];
    }
    double var = sum/(timings.size()-2);
    double sdv = sqrt(var);

    std::cout << "\nwith fastest " << fastest << ", average " << avg << ", stddev " << sdv;
}

double naive(const char *p) {
    double r = 0.0;
    bool neg = false;
    if (*p == '-') {
        neg = true;
        ++p;
    }
    while (*p >= '0' && *p <= '9') {
        r = (r*10.0) + (*p - '0');
        ++p;
    }
    if (*p == '.') {
        double f = 0.0;
        int n = 0;
        ++p;
        while (*p >= '0' && *p <= '9') {
            f = (f*10.0) + (*p - '0');
            ++p;
            ++n;
        }
        r += f / std::pow(10.0, n);
    }
    if (neg) {
        r = -r;
    }
    return r;
}

int main(int argc, char**argv){
	using namespace granite;
	using namespace granite::base;

	// string testing:
	string str("PokeMoN .asd234235534;;'sdf; !");
	lowerCase(str);
	std::cout<<str<<std::endl;

	str="PokeMoN .asd234235534;;'sdf; !";
	upperCase(str);
	std::cout<<str<<std::endl;

	str=" trim whitespaces  \t";
	trimWhitespaces(str);
	std::cout<<"|"<<str<<"|"<<std::endl;

	str="                trim     whitespaces         ";
	trimWhitespaces(str);
	std::cout<<"|"<<str<<"|"<<std::endl;

	str="A value of zero causes the thread to relinquish the remainder of its time slice to any other thread that is ready to run (the)";
	findAndDelete(str,"the");
	std::cout<<str<<std::endl;

	str="A value of zero causes the thread to relinquish the remainder of its time slice to any other thread that is ready to run (the)";
	findAndReplace(str,"the","OMG");
	std::cout<<str<<std::endl;

	str="A value of zero causes the the thread to relinquish the remainder of its time slice to any other thread that is ready to run (the)";
	findAndDelete(str,"the");
	findAndReplace(str,"  "," ");
	findAndReplace(str,"  "," ");
	std::cout<<str<<std::endl;

	str="some very important thing, and not very important one";
	findAndCutAfter(str,"thing");
	std::cout<<str<<std::endl;

	str="some very important thing, and not very important one";
	findAndCutBefore(str,"and not");
	std::cout<<str<<std::endl;

	str="i dont need whitespaces any more :|";
	deleteWhitespaces(str);
	std::cout<<str<<std::endl;

	str="A value of zero causes the the thread to relinquish the remainder of its time slice to any other thread that is ready to run (the)";
	str=containsSubstr(str,"zero")?"contains 'zero'":"!contains 'zero'";
	std::cout<<str<<std::endl;
	findAndDelete(str,"zero");
	str=containsSubstr(str,"zero")?"contains 'zero'":"!contains 'zero'";
	std::cout<<str<<std::endl;

	str="some|parameters|divided|1.0f|by|special|characters"; // empty strings may occur in results!
	std::vector<string> out;
	std::vector<stringRange> out2;
	divideString(str,'|',out);
	divideString(str,'|',out2);
	std::cout<<"printing divided string:\n";
	for(auto &it:out)
		std::cout<<it<<std::endl;
	std::cout<<"printing divided string ranges:\n";
	for(auto &it:out2)
		std::cout<<"range: (begin:"<<it.ibegin(str)<<", end:"<<it.iend(str)<<", count: "<<it.count()<<") string: "<<it.str()<<std::endl<<std::flush;
	std::cout<<"printing divided string: finished\n";

	std::cout<<"similarity: "<<matchString("i dont need whitespaces any more :|","i dont need whitespaces any more :)")<<std::endl;
	std::cout<<"similarity: "<<matchString("i dont need whitespaces any more :|","i dont need any more :)")<<std::endl;
	std::cout<<"similarity: "<<matchString("i dont need whitespaces any more :|","i dont Need wihtespaces any More :|")<<std::endl;
	std::cout<<"similarity: "<<matchString("i dont need whitespaces any more :|","i dont need whitespaces any more :|")<<std::endl;

#ifdef GE_PLATFORM_WINDOWS
	std::cout<<extractFileName("c:\\program files\\GraniteED\\GED.exe")<<std::endl;
	std::cout<<extractFilePath("c:\\program files\\GraniteED\\GED.exe")<<std::endl;
	std::cout<<extractExt("c:\\program files\\GraniteED\\GED.exe")<<std::endl;
	std::cout<<cutLongPath("c:\\program files\\very\\looooooooong\\path\\GraniteED\\GED.exe")<<std::endl;
#else
	std::cout<<extractFileName("/home/calx/GraniteED/GED")<<std::endl;
	std::cout<<extractFilePath("/home/calx/GraniteED/GED")<<std::endl;
	std::cout<<extractExt("/home/calx/GraniteED/GED/src/test.cpp")<<std::endl;
	std::cout<<cutLongPath("/home/calx/GraniteED/GED/src/test.cpp")<<std::endl;
#endif

	std::cout<<strToInt32("-667")<<std::endl;
	std::cout<<strToInt32("6677")<<std::endl;
	std::cout<<strToUInt64("1234567890123")<<std::endl;
	std::cout<<strToFloat("3.141521")<<std::endl;
	std::cout<<strToDouble("3.141521519")<<std::endl;
	std::cout<<strToBool("true")<<std::endl;
	std::cout<<strToBool("truet")<<std::endl;
	std::cout<<strToBool("tru")<<std::endl;
	std::cout<<strToBool("false")<<std::endl;

	std::cout<<"now the same but templated version:"<<std::endl;
	std::cout<<fromStr<int>("-667")<<std::endl;
	std::cout<<fromStr<int32>("6677")<<std::endl;
	std::cout<<fromStr<int64>("1234567890123")<<std::endl;
	std::cout<<fromStr<float>("3.141521")<<std::endl;
	std::cout<<fromStr<double>("3.141521519")<<std::endl;
	std::cout<<fromStr<bool>("true")<<std::endl;
	std::cout<<fromStr<bool>("truet")<<std::endl;
	std::cout<<fromStr<bool>("tru")<<std::endl;
	std::cout<<fromStr<bool>("false")<<std::endl;

	string buff(0,' ');
	buff.resize(20);
	std::cout<<signedToStr<int>(96661,buff).str()<<std::endl;
	std::cout<<signedToStr<int>(-96661,buff).str()<<std::endl;
	std::cout<<signedToStr<int64>(1234567890123,buff).str()<<std::endl;
	std::cout<<signedToStr<int64>(-1234567890,buff).str()<<std::endl;
	std::cout<<unsignedToStr<int>(26668,buff).str()<<std::endl;
	std::cout<<unsignedToStr<int64>(123321000666,buff).str()<<std::endl;
	std::cout<<realToStr<float>(3.141521,buff,7).str()<<std::endl;
	std::cout<<realToStr<double>(3.1415213642,buff,13).str()<<std::endl;
	std::cout<<floatToStr(3.141521)<<std::endl;
	std::cout<<doubleToStr(3.1415213642)<<std::endl;

	std::cout<<"now the same but overloaded:"<<std::endl;
   	std::cout<<toStr(96661,buff).str()<<std::endl;
	std::cout<<toStr(-96661,buff).str()<<std::endl;
	std::cout<<toStr(1234567890123,buff).str()<<std::endl;
	std::cout<<toStr(-1234567890,buff).str()<<std::endl;
	std::cout<<toStr(26668,buff).str()<<std::endl;
	std::cout<<toStr(123321000666,buff).str()<<std::endl;
	std::cout<<toStr(3.141521,buff).str()<<std::endl;
	std::cout<<toStr(3.1415213642,buff).str()<<std::endl;
	std::cout<<toStr(false,buff).str()<<std::endl;
	std::cout<<toStr(true,buff).str()<<std::endl;
	std::cout<<toStr(0)<<std::endl;

	std::cout<<strs("string"," cat, maybe some float: ",2.71f," or ~PI: ",3.14152112345," integers will work too: ",666,", ",-123456," booleans: ",true,", ",false," int64? no problem: ",123456789012345666ll)<<std::endl;
	std::cout<<strf("string cat, maybe some float: % or ~PI: % integers will work too: %, % booleans: %, % int64? no problem: %",2.71f,3.14152112345,666,-123456,true,false,123456789012345666ll)<<std::endl;
	std::cout<<strf("just testing %% operators...%%")<<std::endl;

	// benchmark
	timer::init();
	timer t;

	static const unsigned int N=50000;
	static const unsigned int R=7;

	std::vector<std::string> nums;
    nums.reserve(N);
    for (size_t i=0 ; i<N ; ++i) {
        std::string y;
        if (i & 1) {
            y += '-';
        }
        y += toStr(rand()%1000,buff).str();
        y += '.';
        y += toStr(rand()%999999,buff).str();
        nums.push_back(y);
    }

	{
        double tsum = 0.0;
        std::vector<double> timings;
        timings.reserve(R);
        for (size_t r=0 ; r<R ; ++r) {
			t.reset();
            for (size_t i=0 ; i<nums.size() ; ++i) {
                double x = 0.0;
                x=strToDouble(nums[i]);
                tsum += x;
            }
            timings.push_back(t.timeS());
        }

        std::cout << "\ngranite: ";
        PrintStats(timings);
        std::cout << std::endl;
        std::cout << tsum << std::endl;
    }

	{
        double tsum = 0.0;
        std::vector<double> timings;
        timings.reserve(R);
        for (size_t r=0 ; r<R ; ++r) {
			t.reset();
            for (size_t i=0 ; i<nums.size() ; ++i) {
                double x = naive(nums[i].c_str());
                tsum += x;
            }
            timings.push_back(t.timeS());
        }

        std::cout << "\nnaive: ";
        PrintStats(timings);
        std::cout << std::endl;
        std::cout << tsum << std::endl;
    }

	{
        double tsum = 0.0;
        std::vector<double> timings;
        timings.reserve(R);
        for (size_t r=0 ; r<R ; ++r) {
			t.reset();
            std::istringstream ss;
            for (size_t i=0 ; i<nums.size() ; ++i) {
                ss.str(nums[i]);
                ss.clear();
                double x = 0.0;
                ss >> x;
                tsum += x;
            }
            timings.push_back(t.timeS());
        }

        std::cout << "\nstringstream: ";
        PrintStats(timings);
        std::cout << std::endl;
        std::cout << tsum << std::endl;
    }

	{
        double tsum = 0.0;
        std::vector<double> timings;
        timings.reserve(R);
        for (size_t r=0 ; r<R ; ++r) {
			t.reset();
            for (size_t i=0 ; i<nums.size() ; ++i) {
                double x = 0.0;
                sscanf(nums[i].c_str(), "%lf", &x);
                tsum += x;
            }
            timings.push_back(t.timeS());
        }

        std::cout << "\nsscanf(): ";
        PrintStats(timings);
        std::cout << std::endl;
        std::cout << tsum << std::endl;
    }

	{
        double tsum = 0.0;
        std::vector<double> timings;
        timings.reserve(R);
        for (size_t r=0 ; r<R ; ++r) {
            t.reset();
            for (size_t i=0 ; i<nums.size() ; ++i) {
                double x = strtod(nums[i].c_str(), 0);
                tsum += x;
            }
            timings.push_back(t.timeS());
        }

        std::cout << "\nstrtod(): ";
        PrintStats(timings);
        std::cout << std::endl;
        std::cout << tsum << std::endl;
    }

    {
        double tsum = 0.0;
        std::vector<double> timings;
        timings.reserve(R);
        for (size_t r=0 ; r<R ; ++r) {
			t.reset();
            for (size_t i=0 ; i<nums.size() ; ++i) {
                double x = atof(nums[i].c_str());
                tsum += x;
            }
            timings.push_back(t.timeS());
        }

        std::cout << "\natof(): ";
        PrintStats(timings);
        std::cout << std::endl;
        std::cout << tsum << std::endl;
    }

	return 0;
}
