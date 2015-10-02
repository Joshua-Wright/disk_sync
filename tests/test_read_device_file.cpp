#include <iostream> // needed for cin, cout, endl
#include <fstream>
// #include <cstring>
// #include <ios>
// #include "sha512.h"
#include <linux/fs.h>  // needed for BLKGETSIZE64
#include <sys/ioctl.h> // needed for ioctl()
#include <fcntl.h>     // needed for open()

/*
g++ -Wall -Wextra -O3 -std=gnu++11 ../lib/sha512-64.S ../lib/sha512.cpp ../lib/sha512_digest.cpp test_read_device_file.cpp -o test_read_device_file
g++ -Wall -Wextra -O3 -std=gnu++11 ../lib/sha512-64.S ../lib/sha512.cpp ../lib/sha512_digest.cpp test_read_device_file.cpp -o test_read_device_file
g++ -Wall -Wextra -O3 -std=gnu++11 ../lib/sha512-64.S ../lib/sha512.o   ../lib/sha512_digest.cpp test_read_device_file.cpp -o test_read_device_file
*/

// std::string binary_digest_to_hex(unsigned char* digest) {
//     char buf[2*SHA512::DIGEST_SIZE+1];
//     buf[2*SHA512::DIGEST_SIZE] = 0;
//     for (int i = 0; i < SHA512::DIGEST_SIZE; i++)
//         sprintf(buf+i*2, "%02x", digest[i]);
//     return std::string(buf);
// }

unsigned long long int BLOCKSIZE = 32768;

int main(int argc, char const *argv[])
{
	// if (argc < 2)
	// 	return 1;
	// std::ifstream input_stream  (argv[1], std::ifstream::binary | std::ios::in);
	// unsigned char buf[64];
	// input_stream.read(reinterpret_cast<char*>(buf), 64);
	// std::cout << binary_digest_to_hex(buf) << std::endl;

	int fh = open("/dev/sdc", 0);
	unsigned long int file_size_in_bytes = 0;
	ioctl(fh, BLKGETSIZE64, &file_size_in_bytes);
	std::cout << file_size_in_bytes << std::endl;
	
	std::ifstream input_stream (argv[1], std::ifstream::binary | std::ifstream::ate);
	// read indefinitely so we can see how fast it is
	unsigned char buffer[BLOCKSIZE];
	for (unsigned long long int block=0; ; block++) {
	input_stream.seekg(block * BLOCKSIZE);
    input_stream.read((char*)buffer, BLOCKSIZE);
    // return input_stream.gcount();
	}
	
	return 0;
}