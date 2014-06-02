
// Test FASTCACHE_MUTABLE_DATA mode.
// Read from the cache using 2 threads and make sure we get an exception when the second thread gets a lock.


// Run with 3 threads
#define THREAD_COUNT 3

// Mutable mode
#define FASTCACHE_MUTABLE_DATA

#include <iostream>
#include <string>
#include <boost/shared_ptr.hpp>
#include "../Fastcache.h"
#include "TestClass.h"
#include <time.h>

using namespace std;
using namespace active911;
using boost::shared_ptr;


// The cache
Fastcache<string, TestClass>cache;



int main(int argc, char **argv) {

	// Start test
	cout << "Testing FASTCACHE_MUTABLE_DATA locking..." << flush;

	// Put something in the cache
	{
		shared_ptr<TestClass>obj=shared_ptr<TestClass>(new TestClass("Tom"));
		cache.set("MyDataKey", obj);
	}

	// Get it back from the cache, twice (test exception)
	{
		shared_ptr<TestClass>read=cache.get("MyDataKey");
		assert(read->name=="Tom");

		// Now get another copy (should throw)
		bool exception_thrown=false;
		try {

			shared_ptr<TestClass>read2=cache.get("MyDataKey");

		} catch (FastcacheObjectLocked &e) {

			exception_thrown=true;
		}
		assert(exception_thrown);
	}

	// Now it should be available again
	shared_ptr<TestClass>tom=cache.get("MyDataKey");
	assert(tom->name=="Tom");
	cout << "...OK" << endl;


	// This test should be equivalent, but let's be sure
	cout << "Testing FASTCACHE_MUTABLE_DATA throws if set shared_ptr not out of scope..." << flush;

	// Put something in the cache, and read it right away
	{
		shared_ptr<TestClass>obj=shared_ptr<TestClass>(new TestClass("Dick"));
		cache.set("MyDataKey2", obj);

		bool exception_thrown=false;
		try {

			shared_ptr<TestClass>read=cache.get("MyDataKey2");

		} catch (FastcacheObjectLocked &e) {

			exception_thrown=true;
		}
		assert(exception_thrown);
	}

	// Now it should be available again
	shared_ptr<TestClass>dick=cache.get("MyDataKey2");
	assert(dick->name=="Dick");

	cout << "...OK" << endl;
	return 0;
}













