//
// Created by j0sh on 11/21/15.
//

#ifndef ASSEMBLER_CONFIG_H
#define ASSEMBLER_CONFIG_H

#include <atomic>  // std::atomic_ullong
#include <chrono>

typedef std::chrono::milliseconds ms;
typedef unsigned long long int ullong;

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
    ullong input_size;
};

config_struct *read_config(const int argc, const char **argv);

#endif //ASSEMBLER_CONFIG_H
