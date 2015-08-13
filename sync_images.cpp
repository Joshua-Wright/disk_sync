// # (c) Copyright 2015 Josh Wright
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <cstdio>
#include <set>
#include "lib/sha512.h"
#include "lib/block_device_size.h"
#include "lib/sha512_digest.h"

/*
g++     -Wall -Wextra -std=gnu++11 lib/sha512-64.S lib/sha512.cpp lib/sha512_digest.cpp lib/block_device_size.cpp sync_images.cpp -o sync_images
g++     -Wall -Wextra -std=gnu++11 lib/sha512-64.o lib/sha512.o   lib/sha512_digest.o   lib/block_device_size.o   sync_images.cpp -o sync_images
clang++ -Wall -Wextra -std=gnu++11 lib/sha512-64.S lib/sha512.cpp lib/sha512_digest.cpp lib/block_device_size.cpp sync_images.cpp -o sync_images
clang++ -Wall -Wextra -std=gnu++11 lib/sha512-64.o lib/sha512.o   lib/sha512_digest.o   lib/block_device_size.o   sync_images.cpp -o sync_images
*/

unsigned long long int BLOCKSIZE;

void print(const long long int i) { std::cout << i << std::endl; };
void print(const std::string   i) { std::cout << i << std::endl; };

typedef unsigned char uchar;

std::streamsize read_block(std::ifstream& stream, const long long int& block, uchar* buffer) {
    stream.seekg(block * BLOCKSIZE);
    stream.read((char*)buffer, BLOCKSIZE);
    return stream.gcount();
};
void write_block(std::fstream& stream, const long long int& block, uchar* buffer, std::streamsize length) {
    stream.seekp(block * BLOCKSIZE);
    stream.write((char*)buffer, length);
    stream.flush();
};
void read_hash(std::fstream&   stream, const long long int& block, sha512_digest& buffer) {
    stream.seekg(block * SHA512_DIGEST_SIZE);
    stream.read(buffer, SHA512_DIGEST_SIZE);
};
void write_hash(std::fstream&  stream, const long long int& block, sha512_digest& buffer) {
    stream.seekp(block * SHA512_DIGEST_SIZE);
    stream.write(buffer, SHA512_DIGEST_SIZE);
    stream.flush();
};

int main(int argc, char const *argv[])
{
    using std::string;
    if (argc < 3) return 1;

    BLOCKSIZE = std::stoi(argv[3]);

    unsigned char current_block_buffer[BLOCKSIZE];
    sha512_digest new_hash, existing_hash;

    std::ifstream input_stream (argv[1], std::ifstream::binary | std::ifstream::ate);
    if (!input_stream){
        std::cout << "Could not open input file." << std::endl;
        return 1;
    }
    unsigned long long int input_size ( std::max( 
        (long long int)block_device_size(argv[1]), 
        (long long int)input_stream.tellg()
    ) );
    input_stream.seekg(0);
    // divide in a way that rounds up
    unsigned long long int n_blocks ((input_size+BLOCKSIZE-1)/BLOCKSIZE);

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
    std::set<unsigned long long int> percentages;
    if (n_blocks >= 10)
        for (unsigned long long int i=0; i<n_blocks; i+=(n_blocks/10) ) {
            // print(i);
            percentages.insert(i);
        }
    // for (auto a: percentages)
    //     print(a);

    // in case the last block is not a full block
    std::streamsize rw_size;
    for (unsigned long long int i=0; i<n_blocks; i++) {
        rw_size = read_block(input_stream, i, current_block_buffer);
        read_hash(hash_stream,   i, existing_hash);
        sha512_hash(current_block_buffer, BLOCKSIZE, (uint64_t*)new_hash);
        // print("");
        // print(existing_hash.to_hex_string());
        // print(     new_hash.to_hex_string());
        if (new_hash == existing_hash) {
            // print(i);
            write_block(output_stream, i, current_block_buffer, rw_size);
            write_hash(hash_stream,    i, new_hash);
        }
        // don't need to check because it simply will never find the value
        auto search = percentages.find(i);
        if (search != percentages.end()){
            std::cout << ((*search)*100)/n_blocks <<"%... ";
            // need to sync because we don't print newlines
            std::cout.flush();
        }
    }
    // this print statement prints the final newline that wasn't printed  by the progress bar
    print("done");
    return 0;
}