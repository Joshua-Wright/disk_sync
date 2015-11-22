// # (c) Copyright 2015 Josh Wright
#include <iostream>
#include <fstream>
#include <list>
#include <boost/thread.hpp>
#include <thread>  // threads
#include <fcntl.h>
#include <stropts.h>
#include <linux/fs.h>
#include "config.h"
#include "progress_thread.h"
#include "hash_thread.h"
#pragma ide diagnostic ignored "UnusedImportStatement"

#include "lib/coreutils/lib/config.h" // needed or else u64.h complains

#pragma clang diagnostic pop
#include "lib/coreutils/lib/sha512.h"

/*
g++ -O3 -Wall -Wextra -fpermissive -lpthread -lboost_system -lboost_thread -std=gnu++14 lib/coreutils/lib/sha512.c sync_images.cpp -o sync_images
*/
int main(int argc, char const *argv[]) {

    config_struct *cfg = read_config(argc, argv);


    // make sure we can open the necessary files
    std::ifstream input_stream(cfg->input_file_path, std::ifstream::binary | std::ifstream::ate);
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
    cfg->n_blocks = ((input_size + cfg->blocksize - 1) / cfg->blocksize);

    // make sure we can open the necessary files
    std::fstream hash_stream(cfg->hash_file_path, std::fstream::binary | std::fstream::in | std::fstream::out);
    if (!hash_stream) {
        std::cout << "Could not open hash file. Make one with:" << std::endl;
        std::cout << "truncate -s " << cfg->n_blocks * SHA512_DIGEST_SIZE << " " << cfg->hash_file_path << std::endl;
        return 3;
    }
    std::fstream output_stream(cfg->output_file_path, std::fstream::binary | std::fstream::in | std::fstream::out);
    if (!output_stream) {
        std::cout << "Could not open output file. Make one with:" << std::endl;
        std::cout << "truncate -s " << input_size << " " << cfg->output_file_path << std::endl;
        return 4;
    }

    // the threads have to open their own files
    input_stream.close();
    output_stream.close();
    hash_stream.close();

    cfg->empty_block = new char[cfg->blocksize];
    cfg->empty_hash = new char[SHA512_DIGEST_SIZE];
    memset(cfg->empty_block, 0, cfg->blocksize);
    sha512_buffer(cfg->empty_block, cfg->blocksize, cfg->empty_hash);

    std::list<std::thread> threads;
    for (int n = 0; n < cfg->thread_cout; n++) {
        // constructs them in-place, so we don't have to worry about a temp var or copy-insertion or anything
        threads.emplace_back(&hash_thread, std::ref(*cfg));
    }
    std::thread display_thread;
    if (cfg->do_status_update) {
        display_thread = std::thread(progress_thread, std::ref(*cfg));
    }

    for (auto &t : threads) {
        t.join();
    }

    if (cfg->do_status_update) {
        display_thread.join();
    }
    return 0;
}
