
// Test2
// Key integrity check
// Do 9000 reads and writes, checking key values
// Note: This will be a little slower than it could be since we are doing string manipulations


// Run with 30 threads
#define THREAD_COUNT 30

// Clock source
#define CLOCKSRC CLOCK_MONOTONIC_RAW

#include <iostream>
#include <string>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/detail/atomic_count.hpp>
#include "../Fastcache.h"
#include "TestClass.h"
#include <time.h>
#include <cassert>
#include <sstream>

using namespace std;
using namespace active911;
using boost::shared_ptr;
using boost::thread;



// Counter for thread runloop
boost::detail::atomic_count run(1);

// The cache
Fastcache<string, TestClass>cache;

// Timer
struct timespec start;
struct timespec end;


void load_cache(int thread_number) {

	ostringstream s;

	// Create a cache object 
	for(int n=0; n<300; n++){
		s.str("");
		s << "MyDataKey" << thread_number << "-" << n;
		shared_ptr<TestClass>obj=shared_ptr<TestClass>(new TestClass(s.str()));
		cache.set(s.str(), obj);
	}

}

void read_cache(int thread_number) {

	ostringstream s;

	// Read 
	for(int n=0; n<300; n++){
		s.str("");
		s << "MyDataKey" << thread_number << "-" << n;

		shared_ptr<TestClass>obj=cache.get(s.str());
		assert(obj->name.compare(s.str())==0);
	}

}


int main(int argc, char **argv) {

	// Setup cout
	cout.precision(5);

	// Start load test
	cout << "Object size is " << sizeof(TestClass) << " bytes" << endl;
	cout << "Loading up cache..." << flush;
	clock_gettime(CLOCKSRC, &start);

	// Start threads
	vector<shared_ptr<boost::thread> > threads;
	for(int n=0; n<THREAD_COUNT; n++){

		threads.push_back(shared_ptr<boost::thread>(new boost::thread(&load_cache,n)));
	}

	// Join the threads
	--run;
	for(int n=0; n<THREAD_COUNT; n++){

		threads[n]->join();
	}	

	// End the load test
	clock_gettime(CLOCKSRC, &end);
	double start_time=(double)start.tv_sec+((double)start.tv_nsec/1000000000.0);
	double end_time=(double)end.tv_sec+((double)end.tv_nsec/1000000000.0);
	double elapsed_time=end_time-start_time;

	cout << ( THREAD_COUNT * 300 ) << " objects stored in " << elapsed_time << " sec" << endl;
	threads.clear();


	// Start read test
	cout << "Reading cache..." << flush;
	clock_gettime(CLOCKSRC, &start);

	// Start threads
	for(int n=0; n<THREAD_COUNT; n++){

		threads.push_back(shared_ptr<boost::thread>(new boost::thread(&read_cache,n)));
	}

	// Join the threads
	--run;
	for(int n=0; n<THREAD_COUNT; n++){

		threads[n]->join();
	}	

	// End the load test
	clock_gettime(CLOCKSRC, &end);
	start_time=(double)start.tv_sec+((double)start.tv_nsec/1000000000.0);
	end_time=(double)end.tv_sec+((double)end.tv_nsec/1000000000.0);
	elapsed_time=end_time-start_time;

	cout << ( THREAD_COUNT * 300 ) << " objects read in " << elapsed_time << " sec" << endl;


	return 0;
}













