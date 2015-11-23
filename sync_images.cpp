// # (c) Copyright 2015 Josh Wright
#include <iostream>
#include <fstream>
#include <list>
#include <thread>  // threads
#include "lib/config.h"
#include "lib/progress_thread.h"
#include "lib/hash_thread.h"
#include "lib/immutable.h"
#include "lib/coreutils/lib/config.h" // needed or else u64.h complains
#include "lib/coreutils/lib/sha512.h"

int main(int argc, char const **argv) {

    /*read the config*/
    config_struct *cfg = read_config(argc, argv);

    // make sure we can open the necessary files
    std::ifstream input_stream(cfg->input_file_path, std::ifstream::binary | std::ifstream::ate);
    if (!input_stream) {
        std::cout << "Could not open input file." << std::endl;
        return 1;
    }

    std::fstream hash_stream(cfg->hash_file_path, std::fstream::binary | std::fstream::in | std::fstream::out);
    if (!hash_stream) {
        std::cout << "Could not open hash file. Make one with:" << std::endl;
        std::cout << "truncate -s " << cfg->n_blocks * SHA512_DIGEST_SIZE << " " << cfg->hash_file_path << std::endl;
        return 3;
    }
    std::fstream output_stream(cfg->output_file_path, std::fstream::binary | std::fstream::in | std::fstream::out);
    if (!output_stream) {
        std::cout << "Could not open output file. Make one with:" << std::endl;
        std::cout << "truncate -s " << cfg->input_size << " " << cfg->output_file_path << std::endl;
        return 4;
    }

    // the threads have to open their own files
    input_stream.close();
    output_stream.close();
    hash_stream.close();

    /*unlock the files so they may be modified*/
    set_mutable(cfg->output_file_path.c_str());
    set_mutable(cfg->hash_file_path.c_str());

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

    /*do not join the display thread if no display thread exists*/
    if (cfg->do_status_update) {
        display_thread.join();
    }

    /*lock the files so nobody changes them while we aren't looking*/
    set_immutable(cfg->output_file_path.c_str());
    set_immutable(cfg->hash_file_path.c_str());

    return 0;
}
