

#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/functional/hash.hpp>
#include <boost/detail/atomic_count.hpp>
#include <vector>
#include <map>
#include <iostream>
#include <time.h>

// Shard size.  This should be much larger than the number of threads likely to access the cache at any one time.
#ifndef FASTCACHE_SHARDSIZE
	#define FASTCACHE_SHARDSIZE 256u
#endif

using boost::shared_ptr;
using boost::mutex;


namespace active911 {


template <class Key, class T>
class Fastcache {

	/**
	 * CacheItem
	 * A wrapper class for cache values
	 */
	template <class W>
	class CacheItem {
	public:

		CacheItem(shared_ptr<T> data, time_t expiration){

			this->data=data;
			this->expiration=expiration;
		};

		shared_ptr<T> data;
		time_t expiration;
	};

	template <class S>	// Keep compiler happy... really will be T
	class Shard {
	public:
		Shard(){

			this->guard=shared_ptr<mutex>(new mutex());
		};

		shared_ptr<mutex> guard;
		std::map<Key,shared_ptr<CacheItem<T> > > map;
	};

public:

	Fastcache(){

		// We are making a new cache.  Init our shards.
		this->shards.reserve(FASTCACHE_SHARDSIZE);
		for(unsigned int n=0; n<FASTCACHE_SHARDSIZE; n++) {

			shared_ptr<Shard<T> >p (new Shard<T>());
			this->shards.push_back(p);
		}

		// Start up the curator thread
		this->curator_run=shared_ptr<boost::detail::atomic_count>(new boost::detail::atomic_count(1));
		this->curator=shared_ptr<boost::thread>(new boost::thread(&Fastcache::curate, this));
	};

	~Fastcache(){

		// Retire the curator
		--(*this->curator_run);
		this->curator->join();

	};



	/**
	 * Set a value into the cache
	 *
	 * @param id the key
	 * @param val shared_ptr to the object to set
	 * @param expiration UNIX timestamp
	 */
	void set(Key id, shared_ptr<T> val, time_t expiration=0){

		// Get shard
		size_t index=this->calc_index(id);
		shared_ptr<Shard<T> >shard=this->shards.at(index);

		shared_ptr<CacheItem<T> >item=shared_ptr<CacheItem<T> >(new CacheItem<T>(val, expiration));

		// Lock and write
		mutex::scoped_lock lock(*shard->guard);
		#ifdef FASTCACHE_SLOW
		sleep(1);
		#endif
		shard->map.insert(std::pair<Key,shared_ptr<CacheItem<T> > >(id,item));	//TODO... use .emplace() once we have C++11 !! (may be faster)


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
		shared_ptr<Shard<T> >shard=this->shards.at(index);

		// Lock and read
		mutex::scoped_lock lock(*shard->guard);
		#ifdef FASTCACHE_SLOW
		sleep(1);
		#endif
		shared_ptr<CacheItem<T> >item=shard->map.at(id);			// Will throw std::out_of_range if not there!

		return item->data;
	};

	protected:

	/**
	 * We are the curator
	 *
	 * Purge expired keys, etc
	 */
	void curate(){

		while(*this->curator_run) {

			sleep(1);
		}

	};


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
	std::vector<shared_ptr<Shard<T> > > shards;
	shared_ptr<boost::thread>curator;
	shared_ptr<boost::detail::atomic_count>curator_run;


};








}