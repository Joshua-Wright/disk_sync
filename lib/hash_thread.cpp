//
// Created by j0sh on 11/21/15.
//
#include <cstdio>
#include <stdexcept>
#include <cstring> // memcmp, memset, ...
#include <fcntl.h> // fallocate

#pragma clang diagnostic push
#pragma ide diagnostic ignored "UnusedImportStatement"

#include "../lib/coreutils/lib/config.h" // needed or else u64.h complains

#pragma clang diagnostic pop

#include "../lib/coreutils/lib/sha512.h" // for sha512_buffer
#include "hash_thread.h"

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
                fallocate(fileno(output_file), FALLOC_FL_PUNCH_HOLE || FALLOC_FL_KEEP_SIZE,
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