//
// Created by j0sh on 11/3/15.
//
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
//#include <cstring>
#include "../lib/coreutils/lib/config.h"
#include "../lib/coreutils/lib/sha512.h"

// this is defined in the coreutils header
// idk why it's an enum though
//const int SHA512_DIGEST_SIZE = 64;
using namespace std;

std::string binary_digest_to_hex(unsigned char *digest) {
    std::string output;
    std::stringstream output_stream;
    for (int i = 0; i < SHA512_DIGEST_SIZE; i++) {
        output_stream.setf(std::ios::hex, std::ios::basefield);
        output_stream.unsetf(std::ios::showbase);
        output_stream << std::setfill('0') << std::setw(2) << (int) digest[i];
    }
    output_stream.flush();
    output_stream.sync();
    return output_stream.str();
}

struct hash_test {
    string data;
    string expected_hash;
    unsigned char digest[SHA512_DIGEST_SIZE];

    hash_test(string data, string expected_hash) : data(data), expected_hash(expected_hash) { };

    void verify() {
        sha512_buffer(data.c_str(), data.size(), digest);
        string hash_str = binary_digest_to_hex(digest);
        if (hash_str != expected_hash) {
            cout << "Failed to hash: " << data << endl;
            cout << "Expected: " << expected_hash << endl;
            cout << "Got:      " << hash_str << endl;
        }
    }
};


int main() {
    using namespace std;
    hash_test a("asdf",
                "401b09eab3c013d4ca54922bb802bec8fd5318192b0a75f201d8b3727429080fb337591abd3e44453b954555b7a0812e1081c39b740293f765eae731f5a65ed1");
    hash_test b("test all of this stuff 12345",
                "fcd5f7f2ac2a36472805bb0e0c96efa252b6ea180d7885c8c7d913382103f4f8b49215c4f1d5975d2427c7d9c7d0f1c5f6901df9d07fbd3303b7ac6ab20a0d17");
    a.verify();
    b.verify();
}