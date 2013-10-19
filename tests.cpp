

#include <iostream>
#include <string>
#include <unistd.h>
#include <boost/shared_ptr.hpp>
#include <sys/types.h>
#include "Fastcache.h"

using namespace std;
using namespace active911;
using boost::shared_ptr;

class TestClass {

public:
	TestClass() {

		cout<<"TestClass CONSTR"<<endl;
	};
	~TestClass() {

		cout<<"TestClass DESTR"<<endl;
	};
	int id;
	string name;
};

int main(int argc, char **argv) {

	cout << "Starting" << endl;

	// Make
	shared_ptr<TestClass>in=shared_ptr<TestClass>(new TestClass());
	in->id=23;
	in->name="My name";

	// Store
	Fastcache<string, TestClass>cache;
	cache.set("99",in);

	// Fetch
	shared_ptr<TestClass>out=cache.get("99");

	cout << out->id << ":" << out->name << endl;

	cout << "Ending" << endl;
	return 0;
}