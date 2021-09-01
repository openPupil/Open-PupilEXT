
#include <ctime>
#include <cstdlib>
#include <mutex>

#ifdef _WIN32
	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>
	#include <Dbghelp.h>
	#pragma comment(lib, "Dbghelp.lib")
#elif ! defined(__CYGWIN__) && ! defined(EMSCRIPTEN)
	#include <execinfo.h>
#endif

#ifdef USE_OPENMP
	#include <omp.h>
#endif

#include <spii/spii.h>

namespace spii
{

double wall_time()
{
	#ifdef USE_OPENMP
		return omp_get_wtime();
	#else
		return std::time(nullptr);
	#endif
}

double cpu_time()
{
	return double(std::clock()) / double(CLOCKS_PER_SEC);
}

std::mutex spii_stack_trace_mutex;

std::string get_stack_trace(void)
{
	using namespace std;

	// Absolutely required for the Windows version.
	lock_guard<mutex> lock(spii_stack_trace_mutex);

	const unsigned int max_stack_size = 10000;
	void* stack[max_stack_size];

	string sout = "\n\nStack trace:\n";

	#ifdef _WIN32
		HANDLE process;
		process = GetCurrentProcess();
		SymInitialize(process, NULL, TRUE);

		unsigned short frames = CaptureStackBackTrace(0, max_stack_size, stack, NULL);

		typedef IMAGEHLP_SYMBOL64 SymbolInfo;
		SymbolInfo* symbol = (SymbolInfo *) malloc(sizeof(SymbolInfo) + 256);
		symbol->MaxNameLength = 255;
		symbol->SizeOfStruct = sizeof(SymbolInfo);
	
		for (unsigned int i = 0; i < frames; i++) {
			string symbol_name = "???";
			ptrdiff_t address = ptrdiff_t(stack[i]);
			ptrdiff_t offset = 0;
			if (SymGetSymFromAddr64(process, DWORD64(stack[i]), nullptr, symbol)) {	
				address = ptrdiff_t(symbol->Address);
				symbol_name = symbol->Name;
				offset = ptrdiff_t(stack[i]) - address;
			}

			if (symbol_name.find("spii::get_stack_trace") != symbol_name.npos) {
				continue;
			}
			if (symbol_name.find("spii::verbose_error") != symbol_name.npos) {
				continue;
			}

			sout += to_string(frames - i - 1, ": ", symbol_name, " + ", offset, "\n");
		}
		free(symbol);
	#elif ! defined(__CYGWIN__) && ! defined(EMSCRIPTEN)
		size_t frames = backtrace(stack, max_stack_size);
		char** messages = backtrace_symbols(stack, frames);
		for (unsigned int i = 0; i < frames; i++) {
			std::string symbol_name = messages[i];
			if (symbol_name.find("get_stack_trace") != symbol_name.npos) {
				continue;
			}
			if (symbol_name.find("verbose_error") != symbol_name.npos) {
				continue;
			}

			sout += to_string(frames - i - 1, ": ", symbol_name, ")\n");
		}
		std::free(messages);
	#else
		sout += "<not available>\n";
	#endif
	return sout;
}

// Removes the path from __FILE__ constants and keeps the name only.
std::string extract_file_name(const char* full_file_cstr)
{
	using namespace std;

	// Extract the file name only.
	string file(full_file_cstr);
	auto pos = file.find_last_of("/\\");
	if (pos == string::npos) {
		pos = 0;
	}
	file = file.substr(pos + 1);  // Returns empty string if pos + 1 == length.

	return file;
}

void verbose_error_internal(const char* expr, const char* full_file_cstr, int line, const char* args)
{
	std::stringstream stream;
	stream << "Assumption failed: " << expr << " in " << extract_file_name(full_file_cstr) << ":" << line << ". "
		    << args;
	stream << get_stack_trace();
	throw std::logic_error(stream.str());
}

}
