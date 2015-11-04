// # (c) Copyright 2015 Josh Wright
#include <iostream>
#include <fstream>
#include <list>
#include <cstring> // memcmp, memset ...
#include <ctime> // time
#include <thread>  // threads
#include <unistd.h> // sleep (linux-only)
#include <atomic>  // std::atomic_ullong
#include <fcntl.h> // fallocate?
#include <stropts.h> // fallocate?
#include <linux/fs.h> // fallocate?
#include <iomanip> // setw, setfill

#pragma clang diagnostic push
#pragma ide diagnostic ignored "UnusedImportStatement"

#include "lib/coreutils/lib/config.h" // needed or else u64.h complains

#pragma clang diagnostic pop

#include "lib/coreutils/lib/sha512.h" // for sha512_buffer

/*
g++ -O3 -Wall -Wextra -lpthread -std=gnu++14 lib/coreutils/lib/sha512.c sync_images.cpp -o sync_images -fpermissive
*/

typedef unsigned char uchar;
typedef unsigned long long int ullong;
typedef long long int llong;


const int THREAD_COUNT = 4;
const llong OUTPUT_INTERVAL = 1;

void progress_thread(std::atomic_ullong &current_block_atomic, const ullong n_blocks) {
    time_t start_time = std::time(0);
    long eta;
    long eta_min = 0;
    long eta_sec = 0;
    long eta_h = 0;
    double progress_precentage = 0;
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wfor-loop-analysis"
    while (current_block_atomic < n_blocks) {
        progress_precentage = 1.0 * current_block_atomic / n_blocks;
        eta = (long) ((time(0) - start_time) * (1.0 - progress_precentage) / progress_precentage);
        eta_sec = eta % 60;
        eta_min = (eta / 60) % 60;
        eta_h = (eta / (60 * 60));
        std::cout << "\r                                                   ";
        std::cout.flush();
        std::cout << "\r" << 100 * progress_precentage << "%";
        std::cout << " ETA: ";
        std::cout << eta_h << "h ";
        std::cout << eta_min << "m ";
        std::cout << eta_sec << "s";
        std::cout.flush();
        sleep(OUTPUT_INTERVAL);
    }
#pragma clang diagnostic pop
    // pad output with spaces to make sure that we clear the eta timer
    std::cout << "\rDone                            " << std::endl;
};

void hash_thread(
        std::atomic_ullong &current_block_atomic,
        const std::string &input_filename,
        const std::string &output_filename,
        const ullong n_blocks,
        const ullong blocksize,
        const char *empty_block,
        const uint64_t *empty_hash_uint64_t
) {

    std::string hash_filename = output_filename + ".hash";
    std::FILE *input_file = fopen(input_filename.c_str(), "r+");
    std::FILE *hash_file = fopen(hash_filename.c_str(), "r+");
    std::FILE *output_file = fopen(output_filename.c_str(), "r+");
    std::size_t rw_size;

    char new_hash[SHA512_DIGEST_SIZE];
    char existing_hash[SHA512_DIGEST_SIZE];
    char block_buffer[blocksize];

    // do this funky comparison because we must only interact with current_block_atomic once per loop
    ullong current_block;
    while ((current_block = current_block_atomic.fetch_add(1)) < n_blocks) {
        memset(existing_hash, 0, SHA512_DIGEST_SIZE);
        memset(block_buffer, 0, blocksize);
        std::fseek(input_file, current_block * blocksize, SEEK_SET);
        rw_size = std::fread(block_buffer, 1, blocksize, input_file);
        std::fseek(hash_file, current_block * SHA512_DIGEST_SIZE, SEEK_SET);
        std::fread(existing_hash, 1, SHA512_DIGEST_SIZE, hash_file);

        // check if block is empty, and if so, don't even hash it
        if (memcmp(block_buffer, empty_block, blocksize) == 0) { // block is empty
            // only write sparse blocks if their hash does not match
            if (memcmp(existing_hash, empty_hash_uint64_t, SHA512_DIGEST_SIZE)) {
                // hashes do not match
                // yes, output_file->_fileno is crazy and non-portable...
                fallocate(output_file->_fileno, FALLOC_FL_PUNCH_HOLE || FALLOC_FL_KEEP_SIZE,
                          (__off_t) (current_block * blocksize), (__off_t) blocksize);
                std::fflush(output_file);
                // write hash
                std::fseek(hash_file, current_block * SHA512_DIGEST_SIZE, SEEK_SET);
                std::fwrite(empty_hash_uint64_t, 1, SHA512_DIGEST_SIZE, hash_file);
                std::fflush(hash_file);
            }
        } else {
            sha512_buffer(block_buffer, rw_size, new_hash);

            if (memcmp(existing_hash, new_hash, SHA512_DIGEST_SIZE) != 0) {
                // hashes do not match
                // output file
                std::fseek(output_file, current_block * blocksize, SEEK_SET);
                std::fwrite(block_buffer, 1, rw_size, output_file);
                std::fflush(output_file);
                // hash file
                std::fseek(hash_file, current_block * SHA512_DIGEST_SIZE, SEEK_SET);
                std::fwrite(new_hash, 1, SHA512_DIGEST_SIZE, hash_file);
                std::fflush(hash_file);
            }
        }
        // make sure files are still working
        if (std::ferror(input_file)) {
            throw std::runtime_error("Error accessing input file!");
        }
        if (std::ferror(output_file)) {
            throw std::runtime_error("Error accessing output file!");
        }
        if (std::ferror(hash_file)) {
            throw std::runtime_error("Error accessing hash file!");
        }
    }
    return;
}


int main(int argc, char const *argv[]) {
    if (argc < 3) {
        std::cout << "Requires at least 3 arguments" << std::endl;
        return 1;
    }

    if (std::stoi(argv[3]) < 0) {
        std::cerr << "Cannot have negative blocksize!" << std::endl;
        return 1;
    }
    ullong blocksize = (ullong) std::stoi(argv[3]);

    std::ifstream input_stream(argv[1], std::ifstream::binary | std::ifstream::ate);
    if (!input_stream) {
        std::cout << "Could not open input file." << std::endl;
        return 1;
    }

    // in lined from block device size
    std::string arg_1_str(argv[1]);
    int fh = open(arg_1_str.c_str(), 0);
    unsigned long long int file_size_in_bytes = 0;
    ioctl(fh, BLKGETSIZE64, &file_size_in_bytes);
    ullong input_size = std::max(
            (ullong) file_size_in_bytes,
            (ullong) input_stream.tellg()
    );
    input_stream.seekg(0);
    // divide and round up
    ullong n_blocks((input_size + blocksize - 1) / blocksize);

    std::string hash_filename(argv[2]);
    hash_filename += ".hash";
    std::fstream hash_stream(hash_filename, std::fstream::binary | std::fstream::in | std::fstream::out);
    if (!hash_stream) {
        std::cout << "Could not open hash file. Make one with:" << std::endl;
        std::cout << "truncate -s " << n_blocks * SHA512_DIGEST_SIZE << " " << hash_filename <<
        std::endl;
        return 1;
    }

    std::fstream output_stream(argv[2], std::fstream::binary | std::fstream::in | std::fstream::out);
    if (!output_stream) {
        std::cout << "Could not open output file. Make one with:" << std::endl;
        std::cout << "truncate -s " << input_size << " " << argv[2] << std::endl;
        return 1;
    }

    // the threads have to open their own files
    input_stream.close();
    output_stream.close();
    hash_stream.close();

    std::atomic_ullong current_block(0);
    std::string input_filename(argv[1]);
    std::string output_filename(argv[2]);

    char *empty_block = new char[blocksize];
    uint64_t *empty_hash_uint64_t = new uint64_t[SHA512_DIGEST_SIZE / sizeof(uint64_t)];
    memset(empty_block, 0, blocksize);
    sha512_buffer(empty_block, blocksize, empty_hash_uint64_t);

    std::list<std::thread> threads;
    for (int n = 0; n < THREAD_COUNT; n++) {
        // constructs them in-place, so we don't have to worry about a temp var or copy-insertion or anything
        threads.emplace_back(&hash_thread,
                             std::ref(current_block),
                             std::cref(input_filename),
                             std::cref(output_filename),
                             n_blocks,
                             blocksize,
                             empty_block,
                             empty_hash_uint64_t
        );
    }
    auto display_thread = std::thread(progress_thread, std::ref(current_block), n_blocks);
    for (auto &t : threads) {
        t.join();
    }
    display_thread.join();
    return 0;
}
