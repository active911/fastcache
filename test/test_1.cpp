
// Test1
// Prove that the mutexes are working by causing the cache to block while reading.
// Read from the cache using 2 threads and observe the delay as they wait for each other.

// Slow down the cache
#define FASTCACHE_SLOW

// Run with 3 threads
#define THREAD_COUNT 3

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


void thread_function(int thread_number) {

	// Create a cache object 
	{
		shared_ptr<TestClass>obj=shared_ptr<TestClass>(new TestClass());
//		cout << "Thread " << thread_number << " BEGIN storing data" << endl;
		cout << "w" << flush;
		cache.set("MyDataKey", obj);
//		cout << "Thread " << thread_number << " END storing data" << endl;
		cout << "W" << flush;

//		cout << "Thread " << thread_number << " BEGIN reading data" << endl;
		cout << "r" << flush;
		shared_ptr<TestClass>read=cache.get("MyDataKey");
//		cout << "Thread " << thread_number << " END reading data" << endl;
		cout << "R" << flush;
	}

	while(run) { sleep(1); }
}

int main(int argc, char **argv) {

	// Setup cout
	cout.precision(5);

	// Start test
	cout << "Testing locks..." << flush;
	clock_gettime(CLOCKSRC, &start);

	// Start several threads, each accessing the cache
	vector<shared_ptr<boost::thread> > threads;
	for(int n=0; n<THREAD_COUNT; n++){

		threads.push_back(shared_ptr<boost::thread>(new boost::thread(&thread_function,n)));
	}

	// Join the threads
	--run;
	for(int n=0; n<THREAD_COUNT; n++){

		threads[n]->join();
	}	

	// End the test
	clock_gettime(CLOCKSRC, &end);
	double start_time=(double)start.tv_sec+((double)start.tv_nsec/1000000000.0);
	double end_time=(double)end.tv_sec+((double)end.tv_nsec/1000000000.0);
	double elapsed_time=end_time-start_time;

	if(elapsed_time > (THREAD_COUNT*2)-1) {

		cout << "...OK";

	} else {

		cout << "...FAIL";
	}

	cout << " (" << elapsed_time << " sec)" << endl;
	return 0;
}













