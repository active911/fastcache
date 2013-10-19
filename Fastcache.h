

#include <boost/thread/mutex.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <vector>
#include <map>
#include <iostream>

// Shard size.  This should be much larger than the number of threads likely to access the cache at any one time.
#define FASTCACHE_SHARDSIZE 256u



namespace active911 {


template <class T>
class Fastcache {

	template <class S>
	class Fastshard {
	public:
		//std::map<unsigned int,S> map;
		Fastshard(){

			std::cout << "Construct" << std::endl;
			this->guard=boost::shared_ptr<boost::mutex>(new boost::mutex());
		};

		~Fastshard(){

			std::cout << "Destruct" << std::endl;
		};

	protected:
		boost::shared_ptr<boost::mutex> guard;

	};


public:
	Fastcache(){

		std::cout<<"Fastcache constr"<<std::endl;

		// We are making a new cache.  Init our shards.
		this->shards.reserve(FASTCACHE_SHARDSIZE);
		for(unsigned int n=0; n<FASTCACHE_SHARDSIZE; n++) {

			boost::shared_ptr<Fastshard<T> >p (new Fastshard<T>());
			this->shards.push_back(p);
		}

	};

	std::vector<boost::shared_ptr<Fastshard<T> > > shards;


};








}