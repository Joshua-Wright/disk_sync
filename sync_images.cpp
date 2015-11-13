// # (c) Copyright 2015 Josh Wright
#include <iostream>
#include <fstream>
#include <list>
#include <boost/property_tree/json_parser.hpp> // for parsing cfg files
#include <boost/thread.hpp>
#include <cstring> // memcmp, memset ...
#include <thread>  // threads
#include <chrono>
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
g++ -O3 -Wall -Wextra -fpermissive -lpthread -lboost_system -lboost_thread -std=gnu++14 lib/coreutils/lib/sha512.c sync_images.cpp -o sync_images
*/

typedef unsigned char uchar;
typedef unsigned long long int ullong;
typedef long long int llong;
typedef boost::exception_detail::clone_impl<boost::exception_detail::error_info_injector<boost::property_tree::ptree_bad_data>> cfg_field_not_found;
typedef std::chrono::milliseconds ms;
typedef std::chrono::steady_clock steady_clock;

struct config_struct {
    std::atomic_ullong current_block;
    std::string input_file_path;
    std::string output_file_path;
    std::string hash_file_path;
    ullong n_blocks;
    ullong blocksize;
    char *empty_block;
    char *empty_hash;
    ms output_interval;
    int thread_cout;
    bool use_sparse_output;
    bool do_status_update;
};

void progress_thread(config_struct &cfg) {
    time_t start_time = std::time(0);
    auto next_time = steady_clock::now();
    long eta;
    long eta_min = 0;
    long eta_sec = 0;
    long eta_h = 0;
    double progress_precentage = 0;
    // the loop condition is updated by hash_thread, so we ask clang to stop whining
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wfor-loop-analysis"
    while (cfg.current_block < cfg.n_blocks) {
        next_time += cfg.output_interval;
        progress_precentage = 1.0 * cfg.current_block / cfg.n_blocks;
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
        std::this_thread::sleep_until(next_time);
    }
#pragma clang diagnostic pop
    // pad output with spaces to make sure that we clear the eta timer
    std::cout << "\rDone                            " << std::endl;
};

void hash_thread(config_struct &cfg) {
    std::FILE *input_file = fopen(cfg.input_file_path.c_str(), "r+");
    std::FILE *output_file = fopen(cfg.output_file_path.c_str(), "r+");
    std::FILE *hash_file = fopen(cfg.hash_file_path.c_str(), "r+");
    std::size_t rw_size;

    char new_hash[SHA512_DIGEST_SIZE];
    char existing_hash[SHA512_DIGEST_SIZE];
    char block_buffer[cfg.blocksize];

    // do this funky comparison because we must only interact with current_block once per loop
    ullong current_block;
    while ((current_block = cfg.current_block.fetch_add(1)) < cfg.n_blocks) {
        memset(existing_hash, 0, SHA512_DIGEST_SIZE);
        memset(block_buffer, 0, cfg.blocksize);
        std::fseek(input_file, current_block * cfg.blocksize, SEEK_SET);
        rw_size = std::fread(block_buffer, 1, cfg.blocksize, input_file);
        std::fseek(hash_file, current_block * SHA512_DIGEST_SIZE, SEEK_SET);
        std::fread(existing_hash, 1, SHA512_DIGEST_SIZE, hash_file);

        // check if block is empty, and if so, don't even hash it
        if (cfg.use_sparse_output && memcmp(block_buffer, cfg.empty_block, cfg.blocksize) == 0) { // block is empty
            // only write sparse blocks if their hash does not match
            if (memcmp(existing_hash, cfg.empty_hash, SHA512_DIGEST_SIZE)) {
                // hashes do not match
                // yes, output_file->_fileno is crazy and non-portable...
                fallocate(output_file->_fileno, FALLOC_FL_PUNCH_HOLE || FALLOC_FL_KEEP_SIZE,
                          (__off_t) (current_block * cfg.blocksize), (__off_t) cfg.blocksize);
                std::fflush(output_file);
                // write hash
                std::fseek(hash_file, current_block * SHA512_DIGEST_SIZE, SEEK_SET);
                std::fwrite(cfg.empty_hash, 1, SHA512_DIGEST_SIZE, hash_file);
                std::fflush(hash_file);
            }
        } else {
            sha512_buffer(block_buffer, rw_size, new_hash);

            if (memcmp(existing_hash, new_hash, SHA512_DIGEST_SIZE) != 0) {
                // hashes do not match
                // output file
                std::fseek(output_file, current_block * cfg.blocksize, SEEK_SET);
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

    if (argc < 2) {
        std::cerr << "Must supply config file!" << std::endl;
        return 1;
    }

    config_struct cfg;
    boost::property_tree::ptree test_tree;
    try {
        /* argv[1] is null-terminated "-" C-string. "-" is Linux convention for "read from stdin" */
        if (argv[1][0] == '-' || argv[1][1] == 0) {
            boost::property_tree::json_parser::read_json(std::cin, test_tree);
        } else {
            boost::property_tree::json_parser::read_json(argv[1], test_tree);
        }
        cfg.current_block = 0;
        cfg.input_file_path = test_tree.get<std::string>("input");
        cfg.output_file_path = test_tree.get<std::string>("output");
        cfg.hash_file_path = cfg.output_file_path + ".hash";
        cfg.blocksize = test_tree.get<ullong>("blocksize");
        /* Default values: */
        cfg.output_interval = (ms) 1000;
        cfg.thread_cout = boost::thread::hardware_concurrency();
        cfg.use_sparse_output = true;
        cfg.do_status_update = true;
        /* check for non-default values */
        if (test_tree.get_optional<int>("threads")) {
            cfg.thread_cout = test_tree.get<int>("threads");
        }
        if (test_tree.get_optional<int>("output interval")) {
            cfg.output_interval = (ms) test_tree.get<int>("output interval");
        }
        if (test_tree.get_optional<bool>("sparse output")) {
            cfg.use_sparse_output = test_tree.get<bool>("sparse output");
        }
        if (test_tree.get_optional<bool>("status update")) {
            cfg.do_status_update = test_tree.get<bool>("status update");
        }
    } catch (cfg_field_not_found &e) {
        std::cerr << "Could not parse config file!" << std::endl;
        std::cerr << "Reason: ";
        std::cerr << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Could not parse config file!" << std::endl;
        return 2;
    }
    if (cfg.blocksize < 1) {
        std::cerr << "Config error: blocksize too small!" << std::endl;
        return 1;
    }
    if (cfg.thread_cout < 1) {
        std::cerr << "Config error: need at least one thread!" << std::endl;
        return 1;
    }
    if (cfg.output_interval < ms(0)) {
        std::cerr << "Config error: cannot output at negetive interval!" << std::endl;
        return 1;
    }

    // make sure we can open the necessary files
    std::ifstream input_stream(cfg.input_file_path, std::ifstream::binary | std::ifstream::ate);
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
    cfg.n_blocks = ((input_size + cfg.blocksize - 1) / cfg.blocksize);

    // make sure we can open the necessary files
    std::fstream hash_stream(cfg.hash_file_path, std::fstream::binary | std::fstream::in | std::fstream::out);
    if (!hash_stream) {
        std::cout << "Could not open hash file. Make one with:" << std::endl;
        std::cout << "truncate -s " << cfg.n_blocks * SHA512_DIGEST_SIZE << " " << cfg.hash_file_path << std::endl;
        return 3;
    }
    std::fstream output_stream(cfg.output_file_path, std::fstream::binary | std::fstream::in | std::fstream::out);
    if (!output_stream) {
        std::cout << "Could not open output file. Make one with:" << std::endl;
        std::cout << "truncate -s " << input_size << " " << cfg.output_file_path << std::endl;
        return 4;
    }

    // the threads have to open their own files
    input_stream.close();
    output_stream.close();
    hash_stream.close();

    cfg.empty_block = new char[cfg.blocksize];
    cfg.empty_hash = new char[SHA512_DIGEST_SIZE];
    memset(cfg.empty_block, 0, cfg.blocksize);
    sha512_buffer(cfg.empty_block, cfg.blocksize, cfg.empty_hash);

    std::list<std::thread> threads;
    for (int n = 0; n < cfg.thread_cout; n++) {
        // constructs them in-place, so we don't have to worry about a temp var or copy-insertion or anything
        threads.emplace_back(&hash_thread, std::ref(cfg));
    }
    std::thread display_thread;
    if (cfg.do_status_update) {
        display_thread = std::thread(progress_thread, std::ref(cfg));
    }

    for (auto &t : threads) {
        t.join();
    }

    if (cfg.do_status_update) {
        display_thread.join();
    }
    return 0;
}
