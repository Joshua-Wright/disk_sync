//
// Created by j0sh on 11/21/15.
//

#include <time.h>
#include <iostream>
#include <thread>
#include "progress_thread.h"

typedef std::chrono::steady_clock steady_clock;

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