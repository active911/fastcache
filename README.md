## fastcache

Wicked fast, thread safe in-memory key/object store for C++

### Introduction

Sometimes using an external caching solution like memcached or Redis is just unnecessary. 

What!

No, think about it.  Key/value stores are great if...

- You need to access cached data from several different forward-facing servers
- Your code is written in PHP or something non-persistent (request based)
- You need aging, clustering, etc

On the other hand, what if you...

- Know exactly what kind of data you are caching
- It's only being accessed by one API (think Thrift server)
- Want to store entire objects in their native format
- Want supreme performance


### Impetus

We wrote a miultithreaded server using Apache Thrift in C++.  We then realized that maintaining several connections to Redis was a little silly, because we were working within the framework of a native, persistent, C++ daemon that was already the unified access point for all our API calls.  In short, we had access to the same raw resources as Redis did.  And unlike Redis, we could allow all our happy worker threads to access the data at the same time (Redis is effectively single threaded, BTW).

