// (c) Copyright 2015 Josh Wright
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
// #include <cstdlib>
#include <functional>
#include <thread>
#include <future>
#include <atomic>
#include "../lib/sha512.h"
#include "../lib/sha512_digest.h"
#include <linux/fs.h>  // needed for BLKGETSIZE64
#include <sys/ioctl.h> // needed for ioctl()
#include <fcntl.h>     // needed for open()
/*
g++    -std=gnu++11 -O3 -lpthread ../lib/sha512-64.S ../lib/sha512.cpp ../lib/sha512_digest.cpp test_read_and_hash.cpp -o test_read_and_hash
g++ -g -std=gnu++11 -O0 -lpthread ../lib/sha512-64.S ../lib/sha512.cpp ../lib/sha512_digest.cpp test_read_and_hash.cpp -o test_read_and_hash
./test_read_and_hash
*/
using namespace std;
unsigned long long int BLOCKSIZE = 32768;

// void hash_values(atomic_ullong& current_hash, ifstream input_stream, unsigned long long int max_hashes) {
void hash_values(atomic_ullong& current_hash, const char* input_filename,const unsigned long long int max_hashes) {
	sha512_digest digest;
	unsigned char buffer[BLOCKSIZE];
	std::ifstream input_stream (input_filename, std::ifstream::binary | std::ifstream::ate);
	for (; current_hash<max_hashes; current_hash++) {
		input_stream.seekg(current_hash * BLOCKSIZE);
	    input_stream.read((char*)buffer, BLOCKSIZE);
		sha512_hash(buffer, input_stream.gcount(), digest);
	}
	return;
}



int main(int argc, char const *argv[])
{
	int fh = open(argv[1], 0);
	unsigned long int file_size_in_bytes = 0;
	ioctl(fh, BLKGETSIZE64, &file_size_in_bytes);

	unsigned long long int max_hashes = file_size_in_bytes/BLOCKSIZE;
	
	// std::ifstream input_stream (argv[1], std::ifstream::binary | std::ifstream::ate);
	const char* input_stream = argv[1];
	atomic_ullong num_hash(0);
	thread t1(hash_values, std::ref(num_hash), input_stream, max_hashes );
	thread t2(hash_values, std::ref(num_hash), input_stream, max_hashes );
	thread t3(hash_values, std::ref(num_hash), input_stream, max_hashes );
	thread t4(hash_values, std::ref(num_hash), input_stream, max_hashes );
	thread t5(hash_values, std::ref(num_hash), input_stream, max_hashes );
	t1.join();
	t2.join();
	t3.join();
	t4.join();
	t5.join();
	return 0;
}
