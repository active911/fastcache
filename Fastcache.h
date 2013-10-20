

#include <boost/thread/mutex.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/functional/hash.hpp>
#include <vector>
#include <map>
#include <iostream>

// Shard size.  This should be much larger than the number of threads likely to access the cache at any one time.
#ifndef FASTCACHE_SHARDSIZE
	#define FASTCACHE_SHARDSIZE 256u
#endif

using boost::shared_ptr;
using boost::mutex;


namespace active911 {


template <class Key, class T>
class Fastcache {

	template <class S>	// Keep compiler happy... really will be T
	class Fastshard {
	public:
		Fastshard(){

			this->guard=shared_ptr<mutex>(new mutex());
		};

		/**
		 * Set a value into the map
		 *
		 * @param id the key
		 * @param val shared_ptr to the object to set
		 */
		void set(Key id, shared_ptr<T> val){

			// Lock and write
			mutex::scoped_lock lock(*this->guard);
			#ifdef FASTCACHE_SLOW
			sleep(1);
			#endif
			this->map[id]=val;

		};
		/**
		 * Get a value from the map
		 *
		 * @param id the key
		 * @retval boost::shared_ptr<T>
		 */
		shared_ptr<T> get(Key id){

			// Lock and read
			mutex::scoped_lock lock(*this->guard);
			#ifdef FASTCACHE_SLOW
			sleep(1);
			#endif
			shared_ptr<T>val=this->map.at(id);			// Will throw std::out_of_range if not there!

			return val;
		};

	protected:
		shared_ptr<mutex> guard;
		std::map<Key,shared_ptr<T> > map;

	};


public:

	Fastcache(){

		// We are making a new cache.  Init our shards.
		this->shards.reserve(FASTCACHE_SHARDSIZE);
		for(unsigned int n=0; n<FASTCACHE_SHARDSIZE; n++) {

			shared_ptr<Fastshard<T> >p (new Fastshard<T>());
			this->shards.push_back(p);
		}
	};
	/**
	 * Set a value into the cache
	 *
	 * @param id the key
	 * @param val shared_ptr to the object to set
	 */
	void set(Key id, shared_ptr<T> val){

		// Get shard
		size_t index=this->calc_index(id);
		shared_ptr<Fastshard<T> >shard=this->shards.at(index);

//		std::cout << "ptr is " << index << std::endl;
//		std::cout << "size is" << this->shards.size() << std::endl;

		// Set
		shard->set(id, val);

	};
	/**
	 * Get a value from the cache
	 *
	 * @param id the key
	 * @retval boost::shared_ptr<T>
	 */
	shared_ptr<T> get(Key id){

		// Get shard
		size_t index=this->calc_index(id);
		shared_ptr<Fastshard<T> >shard=this->shards.at(index);

		// Get var
		return shard->get(id);
	};

	protected:

	/**
	 * Calculate the shard index
	 *
	 * It is important that this function has a repeatable but otherwise randomish (uniform) output
	 * We use the boost hash, "a TR1 compliant hash function object"
	 *
	 * @param id 
	 */
	size_t calc_index(Key id){

		return (size_t) this->hash(id) % FASTCACHE_SHARDSIZE;
	};

	boost::hash<Key>hash;
	std::vector<shared_ptr<Fastshard<T> > > shards;


};








}