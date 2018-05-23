#include <base/base.hpp>

int main(int argc, char**argv){
    using namespace granite::base;

    if (!log::init("/home/kuba/log.txt"))
        std::cout << strerror(errno) << std::endl;
    logInfo("logger initialized");
    logInfo("build info: " GE_BUILD_INFO);

    // output something to log
    for (int i = 0; i < 100; ++i) {
        std::stringstream ss;
        ss << "loggser (" << i << ")";
        //log::log(log::logLevelOK, ss.str());
        logOK(ss.str());
        if (70 == i || i == 71 || i == 80)
            log::process();
    }
    
    // print log buffer
    for(size_t i = 0; i < log::getBufferSize(); ++i)
        std::cout << "\nlog entry: " << log::getBuffer(i);
    std::cout << std::endl;
    std::cout.flush();

    logInfo("testing (before crash)");
    gassert(false, "assertion failed?");
    gassert(true, "assertion failed?");

    // just to crash app (to check signal handling)
    int *a = nullptr;
    *a = 666;

    logInfo("logging shutting down...");
    log::shutdown();

    return 0;
}
