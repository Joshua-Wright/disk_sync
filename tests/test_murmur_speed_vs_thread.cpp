//
// Created by j0sh on 11/29/15.
//
#include <cstdio>
#include <vector>
#include <boost/timer/timer.hpp>
#include <list>
#include <thread>
#include <bits/stl_list.h>
#include <iomanip>
#include "../lib/hash_thread.h"
#include "../lib/config.h"
#include "../smhasher-read-only/MurmurHash3.h"

typedef unsigned long long int ullong;
typedef long long int llong;
//const ullong HASH_SIZE_BYTES = 128 / 8;

int main(int argc, char** argv) {
    using namespace std;
    using boost::timer::auto_cpu_timer;
    config_struct cfg;
    cfg.current_block = 0;
    cfg.input_file_path = "/dev/zero";
//    cfg.input_file_path = "/dev/sdb";
//    cfg.output_file_path = "/dev/zero";
//    cfg.hash_file_path = "/dev/zero";
    cfg.output_file_path = "/dev/null";
    cfg.hash_file_path = "/dev/null";
    cfg.input_size = 3ull * 1024ull * 1024ull * 1024ull; /*5 GiB*/
    cfg.blocksize = 32 * 1024; /*32 KiB*/
    cfg.n_blocks = cfg.input_size / cfg.blocksize;
//    cfg.empty_block = ;
//    cfg.empty_hash = ;
//    cfg.output_interval =;
//    cfg.thread_cout = 1;
    cfg.thread_cout = 4;
    cfg.use_sparse_output = false;
    cfg.do_status_update = false;

    /*read parameters, because for some reason doing the testing in C++ for loops produces skewed output*/
    cfg.thread_cout = std::stoi(argv[1]);
    cfg.blocksize = std::stoull(argv[2]);

//    for (cfg.thread_cout = 1; cfg.thread_cout <= 8; cfg.thread_cout++) {
//        cout << "THREADS: " << cfg.thread_cout << endl;
//        for (ullong i = 2 * 1024; i < 1024 * 1024; i *= 4) {
            std::list<std::thread> threads;
//            cfg.blocksize = i;
            cfg.n_blocks = cfg.input_size / cfg.blocksize;
            /*drop caches*/
//            system("sync; echo 3 > /proc/sys/vm/drop_caches");
            sleep(2);
            auto_cpu_timer t;
            for (int n = 0; n < cfg.thread_cout; n++) {
                // constructs them in-place, so we don't have to worry about a temp var or copy-insertion or anything
                threads.emplace_back(&hash_thread, std::ref(cfg));
            }
            for (auto &t : threads) {
                t.join();
            }

            t.stop();
            llong time = t.elapsed().wall;

            long double MB_s = ((long double) cfg.input_size/1024/1024) / (time * 1.0e-9L);
            cout << "Blocksize: " << std::setw(10) << cfg.blocksize << "  " << MB_s << " MB/s" << endl;
//        }
//    }
}