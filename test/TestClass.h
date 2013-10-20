/**
 * Test class
 *
 */

#include <string>

using namespace std;

class TestClass {
public:
	TestClass() {  };
	TestClass(string str) { this->name=str; };
	int id;
	string name;

#ifndef FASTCACHE_TEST_SMALL
	char mem[1008];	// Make it large
#endif
};