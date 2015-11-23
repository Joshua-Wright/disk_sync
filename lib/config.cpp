//
// Created by j0sh on 11/21/15.
//

#include <iostream>
#include <chrono>
//#include <boost/thread/detail/thread.hpp>
#include <boost/thread.hpp>
#include <boost/property_tree/json_parser.hpp> // for parsing cfg files
#include "coreutils/lib/config.h" // needed or else u64.h complains
#include "coreutils/lib/sha512.h"
#include "block_device_size.h"
#include "config.h"

typedef boost::exception_detail::clone_impl<boost::exception_detail::error_info_injector<boost::property_tree::ptree_bad_data>> cfg_field_not_found;

config_struct *read_config(const int argc, const char **argv) {
    if (argc < 2) {
        std::cerr << "Must supply config file!" << std::endl;
        throw std::runtime_error("config File Error");
    }

    /*allocate and return a pointer ot the config struct*/
    config_struct *cfg = new config_struct;
    boost::property_tree::ptree test_tree;
    try {
        /* argv[1] is null-terminated "-" C-string. "-" is Linux convention for "read from stdin" */
        if (argv[1][0] == '-' || argv[1][1] == 0) {
            boost::property_tree::json_parser::read_json(std::cin, test_tree);
        } else {
            boost::property_tree::json_parser::read_json(argv[1], test_tree);
        }
        cfg->current_block = 0;
        cfg->input_file_path = test_tree.get<std::string>("input");
        cfg->output_file_path = test_tree.get<std::string>("output");
        cfg->hash_file_path = cfg->output_file_path + ".hash";
        cfg->blocksize = test_tree.get<ullong>("blocksize");
        /* Default values: */
        cfg->output_interval = (ms) 1000;
        cfg->thread_cout = boost::thread::hardware_concurrency();
        cfg->use_sparse_output = true;
        cfg->do_status_update = true;
        /* check for non-default values */
        if (test_tree.get_optional<int>("threads")) {
            cfg->thread_cout = test_tree.get<int>("threads");
        }
        if (test_tree.get_optional<int>("output interval")) {
            cfg->output_interval = (ms) test_tree.get<int>("output interval");
        }
        if (test_tree.get_optional<bool>("sparse output")) {
            cfg->use_sparse_output = test_tree.get<bool>("sparse output");
        }
        if (test_tree.get_optional<bool>("status update")) {
            cfg->do_status_update = test_tree.get<bool>("status update");
        }
    } catch (cfg_field_not_found &e) {
        std::cerr << "Could not parse config file!" << std::endl;
        std::cerr << "Reason: ";
        std::cerr << e.what() << std::endl;
        throw std::runtime_error("config File Error");
    } catch (...) {
        std::cerr << "Could not parse config file!" << std::endl;
        throw std::runtime_error("config File Error");
    }
    if (cfg->blocksize < 1) {
        std::cerr << "Config error: blocksize too small!" << std::endl;
        throw std::runtime_error("config File Error");
    }
    if (cfg->thread_cout < 1) {
        std::cerr << "Config error: need at least one thread!" << std::endl;
        throw std::runtime_error("config File Error");
    }
    if (cfg->output_interval < ms(0)) {
        std::cerr << "Config error: cannot output at negetive interval!" << std::endl;
        throw std::runtime_error("config File Error");
    }

    /*get the size of the file*/
    cfg->input_size = get_special_file_size(cfg->input_file_path.c_str());
    // divide and round up
    cfg->n_blocks = ((cfg->input_size + cfg->blocksize - 1) / cfg->blocksize);


    cfg->empty_block = new char[cfg->blocksize];
    cfg->empty_hash = new char[SHA512_DIGEST_SIZE];
    memset(cfg->empty_block, 0, cfg->blocksize);
    sha512_buffer(cfg->empty_block, cfg->blocksize, cfg->empty_hash);

    return cfg;
}