#include <cstdio>
#include <vector>
#include <boost/timer/timer.hpp>
#include <bits/stl_numeric.h>
#include "../smhasher-read-only/MurmurHash3.h"

typedef unsigned long long int ullong;
typedef long long int llong;
const ullong HASH_SIZE_BYTES = 128 / 8;

int main() {
    using namespace std;
    using boost::timer::auto_cpu_timer;
    vector<llong> times;
    /*1 GiB*/
    unsigned blocks = 1024 * 1024 * 1024;
    /*a big block of RAM*/
    unsigned char *block_to_hash = new unsigned char[blocks];

    cout << block_to_hash << endl;

    std::memset(block_to_hash, 0xaa, blocks);

    {
        auto_cpu_timer t;
        /*volatile so the loop doesn't get vectorized*/
        for (volatile int i = 0; i < blocks; i += HASH_SIZE_BYTES) {
            MurmurHash3_x64_128(block_to_hash + i, HASH_SIZE_BYTES, 0, block_to_hash + i);
        }
    }
    for (volatile int j = 0; j < 20; j++) { /*hash the hashes because why not?*/
        {
            auto_cpu_timer t;
            /*volatile so the loop doesn't get vectorized*/
            for (int i = 0; i < blocks; i += HASH_SIZE_BYTES) {
                MurmurHash3_x64_128(block_to_hash + i, HASH_SIZE_BYTES, 0, block_to_hash + i);
            }
            times.push_back(t.elapsed().wall);
        }
    }
    /*long double because any int would overflow*/
    long double average_time = std::accumulate(times.begin(), times.end(), 0.0L);
    average_time /= times.size();
    /*convert from nanoseconds to seconds*/
    long double MB_s = 1024.0L / (average_time * 1.0e-9L);
    cout << MB_s << " MB/s" << endl;
}