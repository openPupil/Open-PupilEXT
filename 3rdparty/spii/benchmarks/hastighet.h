//
// Header file for creating benchmarks that allow saving results
// to a baseline file.
//
// Based on Hayai at https://github.com/nickbruun/hayai.
//
#ifndef HASTIGHET_H
#define HASTIGHET_H

#include <fstream>

#ifdef USE_OPENMP
	#include <omp.h>
#else
	#include <ctime>
	namespace hastighet
	{
	inline double omp_get_wtime()
	{
		return time(nullptr);
	}
	}
#endif

#include "Petter-Color.h"

namespace hastighet
{
/// Base test class.

/// @ref SetUp is invoked before each run, and @ref TearDown is invoked
/// once the run is finished. Iterations rely on the same fixture
/// for every run.
///
/// The default test class does not contain any actual code in the
/// SetUp and TearDown methods, which means that tests can inherit
/// this class directly for non-fixture based benchmarking tests.
class Test
{
public:
	virtual ~Test()
	{

	}

	/// Run the test.

	/// @param iterations Number of iterations to gather data for.
	/// @returns the number of seconds the run took.
	double Run()
	{
		using namespace std;

		vector<double> times;
		double total_time = 0;
		do {
			double start_time = omp_get_wtime();
			// Run the test body.
			this->TestBody();
			// Get the ending time.
			double end_time = omp_get_wtime();
			double elapsed_time = end_time - start_time;

			times.push_back(elapsed_time);
			total_time += elapsed_time;
		} while (total_time < 1.0);

		// Return the duration in microseconds.
		return *min_element(times.begin(), times.end());
	}
protected:
	/// Test body.

	/// Executed for each iteration the benchmarking test is run.
	virtual void TestBody()
	{

	}
};



/// Base class for test factory implementations.
class TestFactory
{
public:
	/// Virtual destructor

	/// Has no function in the base class.
	virtual ~TestFactory()
	{

	}


	/// Creates a test instance to run.

	/// @returns a pointer to an initialized test.
	virtual Test* CreateTest() = 0;
};

/// Default test factory implementation.

/// Simply constructs an instance of a the test of class @ref T with no
/// constructor parameters.
///
/// @tparam T Test class.
template<class T>
class TestFactoryDefault
	:   public TestFactory
{
public:
	/// Create a test instance with no constructor parameters.

	/// @returns a pointer to an initialized test.
	virtual Test* CreateTest()
	{
		return new T();
	}
};

/// Test descriptor.

/// Describes the test
class TestDescriptor
{
public:
	/// Initialize a new test descriptor.

	/// @param fixtureName Name of the fixture.
	/// @param testName Name of the test.
	/// @param runs Number of runs for the test.
	/// @param iterations Number of iterations per run.
	/// @param testFactory Test factory implementation for the test.
	TestDescriptor(const char* fixtureName,
					const char* testName,
					TestFactory* testFactory)
		:   FixtureName(fixtureName),
			TestName(testName),
			Factory(testFactory)
	{

	}


	/// Dispose of a test descriptor.
	~TestDescriptor()
	{
		delete this->Factory;
	}


	/// Fixture name.
	std::string FixtureName;


	/// Test name.
	std::string TestName;

	/// Test factory.
	TestFactory* Factory;
};


/// Benchmarking execution controller singleton.
class Benchmarker
{
public:
	/// Get the singleton instance of @ref Benchmarker.

	/// @returns a reference to the singleton instance of the
	/// benchmarker execution controller.
	static Benchmarker& Instance()
	{
		static Benchmarker singleton;
		return singleton;
	}


	/// Register a test with the benchmarker instance.

	/// @param fixtureName Name of the fixture.
	/// @param testName Name of the test.
	/// @param runs Number of runs for the test.
	/// @param iterations Number of iterations per run.
	/// @param testFactory Test factory implementation for the test.
	/// @returns a pointer to a @ref TestDescriptor instance
	/// representing the given test.
	static TestDescriptor* RegisterTest(const char* fixtureName,
										const char* testName,
										TestFactory* testFactory)
	{
		TestDescriptor* descriptor = new TestDescriptor(fixtureName,
														testName,
														testFactory);

		Instance()._tests.push_back(descriptor);

		return descriptor;
	}

	static std::string timeToString(double seconds)
	{
		std::stringstream sout;
		if (seconds > 3600) {
			sout << seconds / 3600 << " h.";
		}
		else if (seconds > 60) {
			sout << seconds / 60 << " min.";
		}
		else if (seconds > 1) {
			sout << seconds << " s.";
		}
		else if (seconds > 1e-3) {
			sout << seconds / 1e-3 << " ms.";
		}
		else if (seconds > 1e-6) {
			sout << seconds / 1e-6 << " us.";
		}
		else {
			sout << seconds / 1e-9 << " ns.";
		}
		return sout.str();
	}

	/// Run all benchmarking tests.
	static void RunAllTests(int argc, char** argv)
	{
		using namespace std;
		using namespace Petter;
		Benchmarker& instance = Instance();

		string baseline_filename = "default.baseline";
		bool save = false;
		for (int a = 1; a < argc; ++a) {
			string arg = argv[a];
			if (arg == "--save") {
				save = true;
				cerr << "Saving new baseline\n";
			}
			else if (!arg.empty() && arg[0] == '-') {
				cerr << "Invalid argument: \"" << arg << "\"" << endl;
				return;
			}
			else {
				baseline_filename = argv[a];
			}
		}

		map<string, double> baselines;
		ifstream fin(baseline_filename.c_str());
		while (fin) {
			string name;
			double seconds;
			fin >> name >> seconds;
			if (fin) {
				baselines[name] = seconds;
			}
		}
		auto results = baselines;

		for (auto itr = instance._tests.begin();
			 itr != instance._tests.end(); ++itr) {
			auto descriptor = *itr;

			string name = descriptor->FixtureName + "."
					   + descriptor->TestName;
			cout << setw(50) << left << name << " " << flush;

			auto test = unique_ptr<Test>(descriptor->Factory->CreateTest());
			double seconds = test->Run();
			results[name] = seconds;

			double relative_time = -1;
			double baseline = -1;
			bool interesting_test = false;

			auto baseline_entry = baselines.find(name);
			if (baseline_entry != baselines.end()) {
				baseline = baseline_entry->second;
				relative_time = seconds / baseline;
				if (relative_time < 0.95 || relative_time > 1.05) {
					interesting_test = true;
				}
			}

			cout << "\r";
			if (interesting_test) {
				cout << WHITE;
			}
			cout << setw(50) << left << name << " " << flush;
			cout << setw(14) << timeToString(seconds);
			if (relative_time >= 0) {
				double current_speed  = 1.0 / seconds;
				double baseline_speed = 1.0 / baseline;

				if (relative_time > 1.05) {
					cout << RED;
					printf("%4.0f%%", 100.0 * (baseline_speed / current_speed - 1.0));
					cout << " slower!";
				}
				else if (relative_time >= 1.0) {
					printf("%4.0f%%", 100.0 * (baseline_speed / current_speed - 1.0));
					cout << " slower.";
				}
				else if (relative_time >= 0.95) {
					printf("%4.0f%%", 100.0 * (current_speed / baseline_speed - 1.0));
					cout << " faster.";
				}
				else if (relative_time > 0.0) {
					cout << GREEN;
					printf("%4.0f%%", 100.0 * (current_speed / baseline_speed - 1.0));
					cout << " faster!";
				}
			}
			cout << NORMAL;
			cout << endl;
		}

		if (save) {
			ofstream fout(baseline_filename.c_str());
			for (auto itr = results.begin(); itr != results.end(); ++itr) {
				fout << itr->first << " " << itr->second << endl;
			}
		}

	}
private:
	/// Private constructor.
	Benchmarker()
	{

	}


	/// Private destructor.
	~Benchmarker()
	{
		// Release all test descriptors.
		std::size_t index = this->_tests.size();
		while (index--)
			delete this->_tests[index];
	}


	std::vector<TestDescriptor*> _tests; ///< Registered tests.
};



}

#define BENCHMARK_CLASS_NAME_(fixture_name, benchmark_name) \
	fixture_name ## _ ## benchmark_name ## _Benchmark

#define BENCHMARK_(fixture_name,										\
				   benchmark_name,					\
				   fixture_class_name)				  \
	class BENCHMARK_CLASS_NAME_(fixture_name, benchmark_name)		 \
		:   public fixture_class_name						   \
	{												   \
	public:				\
		BENCHMARK_CLASS_NAME_(fixture_name, benchmark_name)()		 \
		{												\
																		\
		};								\
	protected:							  \
		virtual void TestBody();										\
	private:															\
		static const ::hastighet::TestDescriptor* _descriptor;		\
	};								  \
																		\
	const ::hastighet::TestDescriptor* BENCHMARK_CLASS_NAME_(fixture_name, benchmark_name)::_descriptor = \
		::hastighet::Benchmarker::Instance().RegisterTest(#fixture_name, \
																	 #benchmark_name, \
																	 new ::hastighet::TestFactoryDefault<BENCHMARK_CLASS_NAME_(fixture_name, benchmark_name)>()); \
																		\
	void BENCHMARK_CLASS_NAME_(fixture_name, benchmark_name)::TestBody()

#define BENCHMARK_F(fixture_name,				  \
					benchmark_name)	   \
	BENCHMARK_(fixture_name,							 \
			   benchmark_name,			   \
			   fixture_name)

#define BENCHMARK(fixture_name,		\
				  benchmark_name)				  \
	BENCHMARK_(fixture_name,							 \
			   benchmark_name,			   \
			   ::hastighet::Test)

#endif

