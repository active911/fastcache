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

	// Make data for cache
	shared_ptr<TestClass>in=shared_ptr<TestClass>(new TestClass());
	in->name="My Test Object";
	shared_ptr<TestClass>in2=shared_ptr<TestClass>(new TestClass());
	in2->name="My Test Object 2";

	// A place to hold cache output
	shared_ptr<TestClass>out;

	// Create cache as string->TestClass store.  Add data
	Fastcache<string, TestClass>cache;
	cache.set("MyDataKey",in);

	// Fetch back, check name
	out=cache.get("MyDataKey");
	cout << "Checking simple object retrieval";
	assert(out);
	assert(out->name=="My Test Object");
	cout << "...OK" << endl;

	// Try to get a fake key
	out=cache.get("NonExistantKey");
	cout << "Checking empty object retrieval";
	assert(!out);
	cout << "...OK" << endl;

	// Check set option FASTCACHE_WRITEMODE_ONLY_WRITE_IF_NOT_SET
	assert(0==cache.set("MyDataKey",in2, 0, FASTCACHE_WRITEMODE_ONLY_WRITE_IF_NOT_SET));	// Wasn't set
	out=cache.get("MyDataKey");
	cout << "Checking FASTCACHE_WRITEMODE_ONLY_WRITE_IF_NOT_SET";
	assert(out);
	assert(out->name=="My Test Object"); // Object 1 (wasn't replaced)
	cout << "...OK" << endl;

	// Check set option FASTCACHE_WRITEMODE_ONLY_WRITE_IF_SET
	assert(0==cache.set("NonExistantKey",in2, 0, FASTCACHE_WRITEMODE_ONLY_WRITE_IF_SET));	// Wasn't set
	out=cache.get("NonExistantKey");
	cout << "Checking FASTCACHE_WRITEMODE_ONLY_WRITE_IF_SET";
	assert(!out);  // Nothing there
	cout << "...OK" << endl;

	// Check set option FASTCACHE_WRITEMODE_WRITE_ALWAYS (default mode)
	assert(cache.set("MyDataKey",in2, 0));	
	out=cache.get("MyDataKey");
	cout << "Checking FASTCACHE_WRITEMODE_WRITE_ALWAYS";
	assert(out);
	assert(out->name=="My Test Object 2"); // Was replaced
	cout << "...OK" << endl;

	// Expiration
	struct timespec time;
	clock_gettime(CLOCK_REALTIME, &time);
	assert(cache.set("MyDataKey2", in2, time.tv_sec+1));	// Expires in 1 sec
	cout << "Checking cache expiration";
	out=cache.get("MyDataKey2");
	assert(out);
	assert(out->name=="My Test Object 2"); 
	sleep(2);
	out=cache.get("MyDataKey2");
	assert(!out);	// Was expired
	cout << "...OK" << endl;

	// Exists and deletion
	cout << "Checking exists()";
	assert(cache.exists("MyDataKey"));
	assert(!cache.exists("NonExistantKey"));
	cache.del("MyDataKey");
	assert(!cache.exists("MyDataKey"));
	cout << "...OK" << endl;

	return 0;
}







