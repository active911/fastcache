## fastcache

Wicked fast, thread safe in-memory key/object store for C++

```
Testing with 30 simultaneous threads
Object size is 16 bytes
Loading up cache...30000 objects stored in 0.027752 sec
Reading cache...30000 objects read in 0.017265 sec
```

### Introduction

Sometimes using an external caching solution like memcached or Redis is just unnecessary. 

What!

No, think about it.  Key/value stores are great if...

- You need to access cached data from several different forward-facing servers
- Your code is written in PHP or something non-persistent (request based)
- You need aging, clustering, etc

On the other hand, what if you...

- Want to store entire objects natively
- Know exactly what kind of data you are caching
- Only need your cache available to one API (like a Thrift server daemon)
- Need multiple worker threads to access the cache
- Want supreme performance


### Impetus

We wrote a miultithreaded server using Apache Thrift in C++.  We then realized that maintaining several connections to Redis was a little silly, because we were working within the framework of a native, persistent, C++ daemon that was already the unified access point for all our API calls.  In short, we had access to the same raw resources as Redis did.  And unlike Redis, we could allow all our happy worker threads to access the data at the same time (Redis is effectively single threaded, BTW).


### Design

fastcache is a template class.  So you can store anything in it, and you can use as a key anything that boost::hash accepts (like int or std::string).  If you replace the hashing function with your own, you can even do away with that requirement.

The design challenge was to not just store data, but to do so in a way that multiple threads could access it concurrently with a minimum of blocking.  Obviously, we could use a single large std::map and then mutex it, but we would have a ton of threads all waiting on each other.  Instead, we shard the cache into M maps.  We then use a hash function to calculate which map (shard) will hold a particular piece of data.  For T concurrent threads reading or writing data, we will have (mostly) non blocking operation as long as our hash generates an even distribution and M >> T.  T defaults to 256 but can be changed by setting FASTCACHE_SHARDSIZE.

Objects are passed to and from the cache wrapped in boost::shared_ptr.  This means that you can get an object from the cache and hold onto it as long as you like without fear of blocking other threads.  If the object is changed in the meantime (i.e., replaced - actual mutations within the cache are not supported!) your copy will remain safe.

Again, DON'T MUTATE anything you put into or take out of the cache!! This is very important to avoid trouble with multiple threads.  
- Create all cache objects using boost::shared_ptr and then let them go out of scope as soon as you set them
- When you get something from the cache, make a copy of the data (not just the pointer) before you do anything except read it

### Example

```cpp
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
```

### Todo

Enforce non mutablility of objects in the cache somehow - perhaps add functions that copy data before and after storage, so as to relieve the user from haing to be careful? OTOH, there would be performance tradeoffs here...

Make Tests!