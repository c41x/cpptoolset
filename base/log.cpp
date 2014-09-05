#include "log.h"
#include <csignal>

namespace granite { namespace base { namespace log {

namespace {

	const size_t bufferSize = 64;
	string v_buffer[bufferSize]; // zamienic na const array (zeby bylo zaalokowane na stercie)
	size_t v_pos = 0; // next index
	size_t v_processBegin = 0; // where to start processing
	size_t v_processCount = 0; // numbers to items to process
	size_t v_size = 0; // size of buffer (if elements count in buffer is smaller than bufferSize)
	std::fstream v_file;

	void signalCallback(int signal) {
		string desc;
		if(signal == SIGABRT) desc = "signal(SIGABRT) abnormal termination condition, as is e.g. initiated by abort()";
		else if(signal == SIGFPE) desc = "signal(SIGFPE) erroneous arithmetic operation such as divide by zero";
		else if(signal == SIGILL)	desc = "signal(SIGILL) invalid program image, such as invalid instruction";
		else if(signal == SIGINT) desc = "signal(SIGINIT) external interrupt, usually initiated by the user";
		else if(signal == SIGSEGV) desc = "signal(SIGSEGV) invalid memory access (segmentation fault)";
		else if(signal == SIGTERM) desc = "signal(SIGTERM) termination request, sent to the program";
		else desc = "signal(?) unknown signal ID";

		// log handling:
		shutdown();
	}

	void installSignal() {
		// maybe use sigaction?
		std::signal(SIGABRT,signalCallback);
		std::signal(SIGFPE,signalCallback);
		std::signal(SIGILL,signalCallback);
		std::signal(SIGINT,signalCallback);
		std::signal(SIGSEGV,signalCallback);
		std::signal(SIGTERM,signalCallback);
	}

	void uninstallSignal() {
		std::signal(SIGABRT, SIG_DFL);
		std::signal(SIGFPE, SIG_DFL);
		std::signal(SIGILL, SIG_DFL);
		std::signal(SIGINT, SIG_DFL);
		std::signal(SIGSEGV, SIG_DFL);
		std::signal(SIGTERM, SIG_DFL);
	}

}

bool init(const string &fileName) {
	v_pos = v_size = v_processBegin = 0;
	installSignal();

	v_file.open(fileName.c_str(), std::ios::out | std::ios::trunc);
	return v_file.good();
}

void log(logLevel level,const string &message) {
	if(level == logLevelOK)
		v_buffer[v_pos] = string("ok: ") + message;
	else if(level == logLevelInfo)
		v_buffer[v_pos] = string("info: ") + message;
	else if(level == logLevelError)
		v_buffer[v_pos] = string("error: ") + message;
	else v_buffer[v_pos] = string("critical: ") + message;

	v_size = std::min(v_size + 1, bufferSize);
	v_pos = (v_pos + 1) % bufferSize;
	v_processCount++;

	if(v_processCount >= bufferSize)
		process(); // we must process to not lost important data (?)
}

void process() {
	while(v_processCount > 0) {
		v_file << v_buffer[v_processBegin % v_size] << std::endl; // v_processCount > 0 hence v_size > 0 too
		v_processBegin = (v_processBegin + 1) % bufferSize;
		v_processCount--;
	}
	v_file.flush();
}

const string &getBuffer(size_t index) {
	return v_buffer[(v_pos + index) % v_size]; // pro circural buffer :)
}

logLevel getLogLevel(size_t index) {
	const char &c = getBuffer(index)[0];
	if(c == 'o') return logLevelOK;
	else if(c == 'i') return logLevelInfo;
	else if(c == 'e') return logLevelError;
	else return logLevelCritical;
}

size_t getBufferSize() {
	return v_size;
}

void shutdown() {
	process();
	v_file.close();
	uninstallSignal();
}

}}}
