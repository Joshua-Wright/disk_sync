// (c) Copyright 2015 Josh Wright
#include <iostream>
#include <iostream>
#include <vector>
#include <string>
#include <functional>
#include <thread>
// #include <future>
#include <atomic>
#include "../lib/sha512.h"
#include "../lib/sha512_digest.h"
/*
g++    -std=gnu++11 -O3 -lpthread ../lib/sha512-64.S ../lib/sha512.cpp ../lib/sha512_digest.cpp time_parallel.cpp -o time_parallel
g++ -g -std=gnu++11 -O3 -lpthread ../lib/sha512-64.S ../lib/sha512.cpp ../lib/sha512_digest.cpp time_parallel.cpp -o time_parallel
./time_parallel
*/
using namespace std;

const char* to_hash = "IrQVBfLhFG9wqKb9JTHXXH426sw3Gapk5bMAzGbkQcD7a5RgEsKpqSqlYjWxzpfOyIBB2OaUe62xzV5yiKmeJO9PjEuNpdoCx0GkdDKGCeZi0WPGXFwtZ8fVkcbjiPaZvTel6QaM4BbrIBHdGokzlXTPRpwZURomK7yOIp5YpDHoXR1f29pwf3OI4IUcT";
const unsigned char* to_hash_uchar = reinterpret_cast<const unsigned char*>(to_hash);
const unsigned long long int MAX_HASHES = 1e7;

// void hash_values(vector<unsigned char*> to_hash,vector<sha512_digest>, )
// int hash_values(atomic_ullong *current_hash) {
void hash_values(atomic_ullong& current_hash) {
	sha512_digest digest;
	// for (;(*current_hash)<MAX_HASHES;(*current_hash)++) {
	for (;current_hash<MAX_HASHES;current_hash++) {
		sha512_hash(to_hash_uchar, 188u, digest);
	}
	// return 1;
	return;
}



int main(int argc, char const *argv[])
{
	using namespace std;
	// vector<unsigned char> to_hash{110, 204, 244, 74, 203, 212, 28, 176, 120, 70, 60, 36, 157, 13, 176, 239, 155, 207, 247, 5 };
	char* to_hash = "IrQVBfLhFG9wqKb9JTHXXH426sw3Gapk5bMAzGbkQcD7a5RgEsKpqSqlYjWxzpfOyIBB2OaUe62xzV5yiKmeJO9PjEuNpdoCx0GkdDKGCeZi0WPGXFwtZ8fVkcbjiPaZvTel6QaM4BbrIBHdGokzlXTPRpwZURomK7yOIp5YpDHoXR1f29pwf3OI4IUcT";
	// vector<future<void>> futures;
	unsigned char* to_hash_uchar = reinterpret_cast<unsigned char*>(to_hash);
	sha512_digest digest;
	// atomic<unsigned long long int> num_hash = 0;
	atomic_ullong num_hash(0);
	// auto handle1 = async(std::launch::async, hash_values, std::ref(num_hash));
	// auto handle2 = async(std::launch::async, hash_values, std::ref(num_hash));
	// auto handle3 = async(std::launch::async, hash_values, std::ref(num_hash));
	// auto handle4 = async(std::launch::async, hash_values, std::ref(num_hash));
	// auto handle5 = async(std::launch::async, hash_values, std::ref(num_hash));
	// handle1.get();
	// handle2.get();
	// handle3.get();
	// handle4.get();
	// handle5.get();
	thread t1(hash_values, std::ref(num_hash));
	thread t2(hash_values, std::ref(num_hash));
	thread t3(hash_values, std::ref(num_hash));
	thread t4(hash_values, std::ref(num_hash));
	// thread t5(hash_values, std::ref(num_hash));
	t1.join();
	t2.join();
	t3.join();
	t4.join();
	// t5.join();
	return 0;
}
