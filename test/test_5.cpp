#define FASTCACHE_CURATOR_SLEEP_MS 100u


#include <iostream>
#include <string>
#include <boost/shared_ptr.hpp>
#include "../Fastcache.h"
#include "TestClass.h"
#include <time.h>


using namespace std;
using namespace active911;
using boost::shared_ptr;


int main(int argc, char **argv) {

	// Create cache as string->TestClass store.  Add data
	Fastcache<int, TestClass>cache;

	// Get time
	struct timespec time;
	clock_gettime(CLOCK_REALTIME, &time);

	cout << "Checking cache culling by curator thread";

	// Insert 100 objects that expire in 1 second
	for(int n=0; n<100; n++){

		cache.set(1000+n,shared_ptr<TestClass>(new TestClass()), time.tv_sec+1);
	}

	// And 100 objects that expire in 2 seconds
	for(int n=0; n<100; n++){

		cache.set(2000+n,shared_ptr<TestClass>(new TestClass()), time.tv_sec+2);
	}

	// And 100 objects that expire in 3 seconds
	for(int n=0; n<100; n++){

		cache.set(3000+n,shared_ptr<TestClass>(new TestClass()), time.tv_sec+3);
	}

	// Check size
	int max_tries=6;
	size_t size=cache.metrics();
	assert(size==300);

	// Watch it decrement
	do {
		size=cache.metrics();
		cout << "..." << size << flush;
		sleep(1);

	} while (size && max_tries--);
	assert(size==0);

	cout << "...OK" << endl;

	return 0;
}







