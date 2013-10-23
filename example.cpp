#include <iostream>
#include <string>
#include <boost/shared_ptr.hpp>
#include "Fastcache.h"

using namespace std;
using namespace active911;
using boost::shared_ptr;

class TestClass {
public:
	int id;
	string name;
};

int main(int argc, char **argv) {

	// Make data for cache
	shared_ptr<TestClass>in=shared_ptr<TestClass>(new TestClass());
	in->id=23;
	in->name="My name";


	// Create cache as string->TestClass store.  Add data
	Fastcache<string, TestClass>cache;
	cache.set("MyDataKey",in);


	// Fetch back
	shared_ptr<TestClass>out=cache.get("MyDataKey");

	cout << out->id << ":" << out->name << endl;

	return 0;
}