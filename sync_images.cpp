// # (c) Copyright 2015 Josh Wright
#include <iostream>
#include <fstream>
#include <string>
#include <list>
#include <cstring>
#include <cstdio>
#include <cstdint>
#include <thread> // for threads
#include <atomic> // for std::atomic_ullong
#include "lib/block_device_size.h"
#include <fcntl.h>
/*
g++        -O3 -Wall -Wextra -lpthread -std=gnu++11 lib/sha512-64.S lib/block_device_size.cpp sync_images.cpp -o sync_images
clang++    -O3 -Wall -Wextra -lpthread -std=gnu++11 lib/sha512-64.S lib/block_device_size.cpp sync_images.cpp -o sync_images
clang++ -g -O0 -Wall -Wextra -lpthread -std=gnu++11 lib/sha512-64.S lib/block_device_size.cpp sync_images.cpp -o sync_images
*/
const int THREAD_COUNT = 4;
// remember to include the assembly sha512 code
extern "C" void sha512_compress(uint64_t state[8], const uint8_t block[128]);
// universal constant
const int SHA512_DIGEST_SIZE = 64; // need to inline stuff from the other code

typedef unsigned char uchar;
typedef unsigned long long int ullong;

void hash_thread(std::atomic_ullong&     current_block_atomic, 
				 const std::string&      input_filename,
				 const std::string&      output_filename,
				 const ullong            n_blocks,
				 const ullong            blocksize,
				 const uchar           * empty_block,
				 const uint64_t        * empty_hash_uint64_t
				 ){

	ullong progress_every_blocks = n_blocks/100;
	std::string hash_filename = output_filename + ".hash";
	std::FILE * input_file  = fopen(input_filename.c_str(),  "r+");
	std::FILE * hash_file   = fopen(hash_filename.c_str(),   "r+");
	std::FILE * output_file = fopen(output_filename.c_str(), "r+");
	std::size_t rw_size;
	uint64_t new_hash_uint64_t      [SHA512_DIGEST_SIZE/sizeof(uint64_t)];
	uint64_t existing_hash_uint64_t [SHA512_DIGEST_SIZE/sizeof(uint64_t)];
	uchar block_buffer_uchar        [blocksize];

	// do this funky comparison because we must only interact with current_block_atomic once per loop
	for (ullong current_block=0; (current_block=current_block_atomic++)<n_blocks; ) {
		std::fseek(input_file, current_block * blocksize, SEEK_SET);
		rw_size = std::fread(block_buffer_uchar, 1, blocksize, input_file);
		std::fseek(hash_file, current_block * SHA512_DIGEST_SIZE, SEEK_SET);
		std::fread(existing_hash_uint64_t, 1, SHA512_DIGEST_SIZE, hash_file);

		// check if block is empty, and if so, don't even hash it
		if (memcmp(block_buffer_uchar, empty_block, blocksize) == 0) { // block is empty
			// only write sparse blocks if their hash does not match
			if (memcmp(existing_hash_uint64_t, empty_hash_uint64_t, SHA512_DIGEST_SIZE)) { // hashes to not match
				// yes, output_file->_fileno is dirty and crazy non-portable...
				fallocate(output_file->_fileno, FALLOC_FL_PUNCH_HOLE || FALLOC_FL_KEEP_SIZE, current_block * blocksize, blocksize);
				// write hash
				std::fseek(hash_file, current_block * SHA512_DIGEST_SIZE, SEEK_SET);
				std::fwrite(empty_hash_uint64_t, 1, SHA512_DIGEST_SIZE, hash_file);
			}
		} else {
			// copy-pasted from sha512_hash function
			new_hash_uint64_t[0] = UINT64_C(0x6A09E667F3BCC908);
			new_hash_uint64_t[1] = UINT64_C(0xBB67AE8584CAA73B);
			new_hash_uint64_t[2] = UINT64_C(0x3C6EF372FE94F82B);
			new_hash_uint64_t[3] = UINT64_C(0xA54FF53A5F1D36F1);
			new_hash_uint64_t[4] = UINT64_C(0x510E527FADE682D1);
			new_hash_uint64_t[5] = UINT64_C(0x9B05688C2B3E6C1F);
			new_hash_uint64_t[6] = UINT64_C(0x1F83D9ABFB41BD6B);
			new_hash_uint64_t[7] = UINT64_C(0x5BE0CD19137E2179);
			uint32_t i;
			for (i = 0; rw_size - i >= 128; i += 128)
				sha512_compress(new_hash_uint64_t, block_buffer_uchar + i);
			uint8_t block[128];
			uint32_t rem = rw_size - i;
			memcpy(block, block_buffer_uchar + i, rem);
			block[rem] = 0x80;
			rem++;
			if (128 - rem >= 16)
				memset(block + rem, 0, 120 - rem);
			else {
				memset(block + rem, 0, 128 - rem);
				sha512_compress(new_hash_uint64_t, block);
				memset(block, 0, 120);
			}
			uint64_t longLen = ((uint64_t)rw_size) << 3;
			for (i = 0; i < 8; i++)
				block[128 - 1 - i] = (uint8_t)(longLen >> (i * 8));
			sha512_compress(new_hash_uint64_t, block);


			if (memcmp(existing_hash_uint64_t, new_hash_uint64_t, SHA512_DIGEST_SIZE) != 0) { // hashes to not match
				std::fseek(output_file, current_block * blocksize, SEEK_SET);
				std::fwrite(block_buffer_uchar, 1, rw_size, output_file);
				std::fseek(hash_file, current_block * SHA512_DIGEST_SIZE, SEEK_SET);
				std::fwrite(new_hash_uint64_t, 1, SHA512_DIGEST_SIZE, hash_file);
			}
		}
		if ( (current_block % progress_every_blocks) == 0) {
				// "\r" returns the terminal cursor to the beginning of the line
				std::cout << "\r" << (current_block*100)/n_blocks << "%";
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
	ullong n_blocks (( input_size + blocksize - 1 )/blocksize);

	std::string hash_filename (argv[2]);
	hash_filename += ".hash";
	std::fstream hash_stream (hash_filename, std::fstream::binary  | std::fstream::in | std::fstream::out);
	if (!hash_stream) {
		std::cout << "Could not open hash file. Make one with:" << std::endl;
		std::cout << "truncate -s " << n_blocks * SHA512_DIGEST_SIZE << " " << hash_filename <<
			std::endl;
		return 1;
	}

	std::fstream output_stream (argv[2], std::fstream::binary|std::fstream::in|std::fstream::out);
	if (!output_stream) {
		std::cout << "Could not open output file. Make one with:" << std::endl;
		std::cout << "truncate -s " << input_size << " " << argv[2] << std::endl;
		return 1;
	}
	
	// the threads have to open their own files
	input_stream.close();
	output_stream.close();
	hash_stream.close();

	// TOOD: Pack this all in a struct?
	std::atomic_ullong     current_block (0);
	std::string            input_filename  (argv[1]);
	std::string            output_filename (argv[2]);
	
	uchar*    empty_block         = new uchar    [blocksize];
	uint64_t* empty_hash_uint64_t = new uint64_t [SHA512_DIGEST_SIZE/sizeof(uint64_t)];
	memset(empty_block, 0, blocksize);
	empty_hash_uint64_t[0] = UINT64_C(0x6A09E667F3BCC908);
	empty_hash_uint64_t[1] = UINT64_C(0xBB67AE8584CAA73B);
	empty_hash_uint64_t[2] = UINT64_C(0x3C6EF372FE94F82B);
	empty_hash_uint64_t[3] = UINT64_C(0xA54FF53A5F1D36F1);
	empty_hash_uint64_t[4] = UINT64_C(0x510E527FADE682D1);
	empty_hash_uint64_t[5] = UINT64_C(0x9B05688C2B3E6C1F);
	empty_hash_uint64_t[6] = UINT64_C(0x1F83D9ABFB41BD6B);
	empty_hash_uint64_t[7] = UINT64_C(0x5BE0CD19137E2179);
	uint32_t i;
	for (i = 0; blocksize - i >= 128; i += 128)
		sha512_compress(empty_hash_uint64_t, empty_block + i);
	uint8_t block[128];
	uint32_t rem = blocksize - i;
	memcpy(block, empty_block + i, rem);
	block[rem] = 0x80;
	rem++;
	if (128 - rem >= 16)
		memset(block + rem, 0, 120 - rem);
	else {
		memset(block + rem, 0, 128 - rem);
		sha512_compress(empty_hash_uint64_t, block);
		memset(block, 0, 120);
	}
	uint64_t longLen = ((uint64_t)blocksize) << 3;
	for (i = 0; i < 8; i++)
		block[128 - 1 - i] = (uint8_t)(longLen >> (i * 8));
	sha512_compress(empty_hash_uint64_t, block);

	std::list<std::thread> threads;
	for (int i=0; i<THREAD_COUNT; i++){
		// constructs them in-place, so we don't have to worry about a temp var or copy-insertion or anything
		threads.emplace_back(hash_thread,
							 std::ref(current_block),
							 std::ref(input_filename),
							 std::ref(output_filename),
							 n_blocks,
							 blocksize,
							 empty_block,
							 empty_hash_uint64_t
							);
	}
	for (auto& t : threads) {
		t.join();
	}
	std::cout << "\rDone   " << std::endl;
	// std::cout << std::endl;
	return 0;
}