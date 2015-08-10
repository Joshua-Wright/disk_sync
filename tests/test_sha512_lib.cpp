#include <iostream>
#include "../lib/sha512.h"

/*
g++ -Wall -Wextra -std=gnu++11 ../lib/sha512.cpp test_sha512_lib.cpp -o test_sha512_lib
g++ -Wall -Wextra -std=gnu++11 ../lib/sha512.o   test_sha512_lib.cpp -o test_sha512_lib
*/ 

using std::string;
using std::cout;
using std::endl;


std::string binary_digest_to_hex(unsigned char* digest) {
    char buf[2*SHA512::DIGEST_SIZE+1];
    buf[2*SHA512::DIGEST_SIZE] = 0;
    for (int i = 0; i < SHA512::DIGEST_SIZE; i++)
        sprintf(buf+i*2, "%02x", digest[i]);
    return std::string(buf);
}

 
int main(int argc, char *argv[])
{
    // unsigned char digest[64];
    sha512_digest digest[2];
    char* input = "asdf";
    unsigned char* input_1 = reinterpret_cast<unsigned char*>(input);
    input = "qwerty";
    unsigned char* input_2 = reinterpret_cast<unsigned char*>(input);
    sha512(input_1, 4u, digest[0]);
    sha512(input_2, 6u, digest[1]);
    cout << "sha512: "<< input_1 << endl << binary_digest_to_hex(digest[0]) << endl;
    cout << "sha512: "<< input_2 << endl << binary_digest_to_hex(digest[1]) << endl;
    return 0;
}