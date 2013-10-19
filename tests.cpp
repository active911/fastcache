

#include <iostream>
#include <string>
#include <unistd.h>
#include <boost/shared_ptr.hpp>
#include <sys/types.h>
#include "Fastcache.h"
using namespace std;
using namespace active911;

class TestClass {

	int id;
	string name;
};

int main(int argc, char **argv) {

	cout << "Starting" << endl;

	//boost::shared_ptr<Fastcache<int> >cache(new Fastcache<int>());

	Fastcache<TestClass>cache;


	cout << "Ending" << endl;
	return 0;
}