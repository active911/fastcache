

#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/functional/hash.hpp>
#include <boost/detail/atomic_count.hpp>
#include <vector>
//#include <iterator>
#include <map>
#include <iostream>
//#include <utility>
#include <time.h>

// Shard size.  This should be much larger than the number of threads likely to access the cache at any one time.
#ifndef FASTCACHE_SHARDSIZE
	#define FASTCACHE_SHARDSIZE 256u
#endif

#ifndef FASTCACHE_CURATOR_SLEEP_MS
	#define FASTCACHE_CURATOR_SLEEP_MS 30000u
#endif

using boost::shared_ptr;
using boost::mutex;


namespace active911 {

// Write modes
enum fastcache_writemode {

	FASTCACHE_WRITEMODE_WRITE_ALWAYS,
	FASTCACHE_WRITEMODE_ONLY_WRITE_IF_SET,
	FASTCACHE_WRITEMODE_ONLY_WRITE_IF_NOT_SET
};	


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

		/**
		 * Have we expired?
		 * 
		 * @retval bool
		 */
		bool expired(){

			// If we have no expiration, the answer is easy
			if(this->expiration==0) {

				return false;
			}

			// Get the time and compare
			struct timespec time;
			clock_gettime(CLOCK_REALTIME, &time);
			return (time.tv_sec > this->expiration);
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

		void cull_expired_keys() {

			// Iterate map.  Somehow this is ok to do even though we are deleting stuff?
			for(typename std::map<Key,shared_ptr<CacheItem<T> > >::iterator it=this->map.begin();it != this->map.end();/* no increment */){

				shared_ptr<CacheItem<T> >item=it->second;

				if(item->expired()){

					this->map.erase(it++);

				} else {

					++it;
				}
			}

		}

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
		this->curator->interrupt();
		this->curator->join();

	};

	/**
	 * Get some metrics
	 *
	 * @retval 
	 */
	size_t metrics() {

		size_t total_size=0;

		// Iterate all objects in cache
		for(typename std::vector<shared_ptr<Shard<T> > >::iterator it=this->shards.begin(); it != this->shards.end(); ++it) {

			shared_ptr<Shard<T> >shard=*it;

			{	// Scope for lock
				mutex::scoped_lock lock(*shard->guard);

				//  tally
				total_size+=shard->map.size();
			}

		}

		return total_size;

	};

	/**
	 * Set a value into the cache
	 *
	 * @param id the key
	 * @param val shared_ptr to the object to set
	 * @param expiration UNIX timestamp
	 * @param mode the write mode
	 * @retval number of items written
	 */
	size_t set(Key id, shared_ptr<T> val, time_t expiration=0, const fastcache_writemode mode=FASTCACHE_WRITEMODE_WRITE_ALWAYS){

		// Get shard
		size_t index=this->calc_index(id);
		shared_ptr<Shard<T> >shard=this->shards.at(index);

		shared_ptr<CacheItem<T> >item=shared_ptr<CacheItem<T> >(new CacheItem<T>(val, expiration));

		// Lock and write
		mutex::scoped_lock lock(*shard->guard);
		#ifdef FASTCACHE_SLOW
		sleep(1);
		#endif

		if(mode==FASTCACHE_WRITEMODE_ONLY_WRITE_IF_SET) {

			if(shard->map.find(id) == shard->map.end()) {			

				// Key not found.  Return.
				return 0;	

			} else {

				// Erase it so we can set it below
				shard->map.erase(id);				
			}
		} 

		std::pair<typename std::map<Key, shared_ptr<CacheItem<T> > >::iterator,bool>result;
		result=shard->map.insert(std::pair<Key,shared_ptr<CacheItem<T> > >(id,item));	//TODO... use .emplace() once we have C++11 !! (may be faster)
		if(!result.second) {

			// Key exists, so nothing was written
			if(mode==FASTCACHE_WRITEMODE_ONLY_WRITE_IF_NOT_SET){

				return 0;

			} else {

				// Erase and re-write
				shard->map.erase(id);
				shard->map.insert(std::pair<Key,shared_ptr<CacheItem<T> > >(id,item));
				return 1;
			}
		}

		return 1;
	};

	/**
	 * Find if a key exists
	 *
	 * @param id the key
	 * @retval 1 if the key exists, 0 otherwise
	 */
	size_t exists(Key id){

		// Get shard
		size_t index=this->calc_index(id);
		shared_ptr<Shard<T> >shard=this->shards.at(index);

		// Lock and check
		mutex::scoped_lock lock(*shard->guard);
		return (shard->map.find(id) == shard->map.end())?0:1;

	};

	/**
	 * Delete a value from the cache
	 *
	 * @param id the key
	 * @retval the number of items erased
	 */
	size_t del(Key id){

		// Get shard
		size_t index=this->calc_index(id);
		shared_ptr<Shard<T> >shard=this->shards.at(index);

		// Lock and erase
		mutex::scoped_lock lock(*shard->guard);
		return shard->map.erase(id);

	};

	/**
	 * Get a value from the cache
	 *
	 * Does not throw for invalid keys (returns empty pointer)
	 *
	 * @param id the key
	 * @retval boost::shared_ptr<T>.  ==empty pointer if nonexistent or expired.
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
		try {
	
			shared_ptr<CacheItem<T> >item=shard->map.at(id);			// Will throw std::out_of_range if not there!

			// Check for expired
			if(item->expired()){

				return shared_ptr<T>();
			}

			// Success. 
			return item->data;

		} catch (std::exception& e) {}

		return shared_ptr<T>();		// Empty
	};

	protected:

	/**
	 * We are the curator
	 *
	 * Purge expired keys, etc
	 */
	void curate(){

		while(*this->curator_run) {

			try {

				// Snooze a little
				boost::this_thread::sleep(boost::posix_time::milliseconds(FASTCACHE_CURATOR_SLEEP_MS));

				// Iterate all objects in cache, checking for expired objects
				for(typename std::vector<shared_ptr<Shard<T> > >::iterator it=this->shards.begin(); it != this->shards.end(); ++it) {

					shared_ptr<Shard<T> >shard=*it;

					{	// Scope for lock
						mutex::scoped_lock lock(*shard->guard);

						// Cull expired keys
						shard->cull_expired_keys();
					}

				}

			} catch(boost::thread_interrupted& e) {

				// We were asked to leave?
				return;

			} catch(std::exception& e) {

				// TODO... something bad happened in the curator thread
			}
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