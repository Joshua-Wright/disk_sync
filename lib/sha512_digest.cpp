#include "sha512_digest.h"
#include <cstring>
#include <cstdint>
#include <sstream>
#include <string>
#include <iomanip>

/*
clang++ -Wall -Wextra -std=gnu++11 -c sha512_digest.cpp
*/

sha512_digest::sha512_digest() {
	memset(digest, 0, digest_size_char);
}

sha512_digest::sha512_digest(unsigned long long int* input) {
	memcpy(input, digest, digest_size_char);
}

sha512_digest::sha512_digest(unsigned char* input) {
	memcpy(input, digest, digest_size_char);
}

bool sha512_digest::operator==(sha512_digest &rhs) {
	return memcmp(digest, (unsigned long long int*)rhs, digest_size_char);
}

std::string sha512_digest::to_hex_string() {
    std::ostringstream buf;
    for (int i=0; i<8; i++)
        buf << std::hex << std::setw(16) << std::setfill('0') << digest[i];
    return buf.str();
}