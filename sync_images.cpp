// # (c) Copyright 2015 Josh Wright
#include <iostream>
#include <fstream>
#include <string>
// #include <vector>
#include <list>
#include <cstring>
#include <cstdio>
#include <set>
#include <thread> // for threads
#include <atomic> // for std::atomic_ullong
#include "lib/sha512.h"
#include "lib/block_device_size.h"
#include "lib/sha512_digest.h"

/*
g++        -O3 -Wall -Wextra -lpthread -std=gnu++11 lib/sha512-64.S lib/sha512.cpp lib/sha512_digest.cpp lib/block_device_size.cpp sync_images.cpp -o sync_images
clang++    -O3 -Wall -Wextra -lpthread -std=gnu++11 lib/sha512-64.S lib/sha512.cpp lib/sha512_digest.cpp lib/block_device_size.cpp sync_images.cpp -o sync_images
clang++ -g -O0 -Wall -Wextra -lpthread -std=gnu++11 lib/sha512-64.S lib/sha512.cpp lib/sha512_digest.cpp lib/block_device_size.cpp sync_images.cpp -o sync_images
*/

const int THREAD_COUNT = 4;

void print(const long long int i){
	std::cout << i << std::endl;
	return;
};
void print(const std::string& i){
	std::cout << i << std::endl;
	return;
};

typedef unsigned char uchar;
typedef unsigned long long int ullong;

void hash_thread(std::atomic_ullong&     current_block_atomic, 
				 const std::string&      input_filename,
				 const std::string&      output_filename,
				 const ullong            n_blocks,
				 const ullong            blocksize,
				 const std::set<ullong>& percentages
				 ){

	std::string hash_filename = output_filename + ".hash";
	std::ifstream   input_stream  (input_filename,  std::ifstream::binary | std::ifstream::ate);
	std::fstream    hash_stream   (hash_filename,   std::fstream::binary  | std::fstream::in | std::fstream::out);
	std::fstream    output_stream (output_filename, std::fstream::binary  | std::fstream::in | std::fstream::out);
	std::streamsize rw_size;
	uchar current_block_buffer[blocksize];
	sha512_digest new_hash, existing_hash;
	
	// do this funky comparison because we must only interact with current_block_atomic once per loop
	for (ullong current_block=0; (current_block=current_block_atomic++)<n_blocks; ) {
		input_stream.seekg(current_block * blocksize);
		input_stream.read((char*)current_block_buffer, blocksize);
		rw_size = input_stream.gcount();

		hash_stream.seekg(current_block * SHA512_DIGEST_SIZE);
		hash_stream.read(existing_hash, SHA512_DIGEST_SIZE);
		
		sha512_hash(current_block_buffer, blocksize, new_hash);

		if (new_hash != existing_hash) {

			output_stream.seekp(current_block * blocksize);
			output_stream.write((char*)current_block_buffer, rw_size);
			output_stream.flush(); // idk if this is a good idea?
		
			hash_stream.seekp(current_block * SHA512_DIGEST_SIZE);
			hash_stream.write(new_hash, SHA512_DIGEST_SIZE);
			hash_stream.flush();
		}
		// don't need to check because it simply will never find the value
		auto search = percentages.find(current_block);
		if (search != percentages.end()){
			std::cout << ((*search)*100)/n_blocks << "%... ";
			// need to sync because we don't print newlines
			std::cout.flush();
		}
	}
	return;
}


int main(int argc, char const *argv[])
{
	if (argc < 3) {
		std::cout << "Requires at least 3 arguments" << std::endl;
		return 1;
	}

	ullong blocksize = std::stoi(argv[3]);

	std::ifstream input_stream (argv[1], std::ifstream::binary | std::ifstream::ate);
	if (!input_stream){
		std::cout << "Could not open input file." << std::endl;
		return 1;
	}
	ullong input_size = std::max( 
		                         (long long int)block_device_size(argv[1]), 
		                         (long long int)input_stream.tellg()
	                            );
	input_stream.seekg(0);
	// divide in a way that rounds up
	ullong n_blocks ((input_size+blocksize-1)/blocksize);

	std::string hash_filename (argv[2]);
	hash_filename += ".hash";
	std::fstream hash_stream (hash_filename, std::fstream::binary  | std::fstream::in | std::fstream::out);
	if (!hash_stream) {
		// does not automatically create a new hash file because missing an output 
		// file is a possible symptom of further failure
		std::cout << "Could not open hash file. Make one with:" << std::endl;
		std::cout << "truncate -s " << n_blocks * SHA512_DIGEST_SIZE << " " << hash_filename <<
			std::endl;
		return 1;
	}

	std::fstream output_stream (argv[2], std::fstream::binary|std::fstream::in|std::fstream::out);
	if (!output_stream) {
		// does not automatically create a new output file because missing an output 
		// file is a possible symptom of further failure
		std::cout << "Could not open output file. Make one with:" << std::endl;
		std::cout << "truncate -s " << input_size << " " << argv[2] << std::endl;
		return 1;
	}

	// build a percentage list only if we have enough blocks
	std::set<ullong> percentages;
	if (n_blocks >= 10){   
		for (ullong i=0; i<n_blocks; i+=(n_blocks/10) ) {
			// print(i);
			percentages.insert(i);
		}
	}

	// New multi-threaded code:
	
	// the threads have to open their own files
	input_stream.close();
	output_stream.close();
	hash_stream.close();
	std::atomic_ullong     current_block (0);
	std::string            input_filename  (argv[1]);
	std::string            output_filename (argv[2]);
	
	std::list<std::thread> threads;
	for (int i=0; i<THREAD_COUNT; i++){
		// constructs them in-place, so we don't have to worry about a temp var or copy-insertion or anything
		threads.emplace_back(hash_thread,
							 std::ref(current_block),
							 std::ref(input_filename),
							 std::ref(output_filename),
							 n_blocks,
							 blocksize,
							 std::ref(percentages)
							);
	}
	for (auto& t : threads) {
		t.join();
	}
	// this print statement prints the final newline that wasn't printed by the progress bar
	print("done");
	return 0;
}