#include <iostream> // needed for cin, cout, endl
// #include <fstream>
// #include <cstring>
// #include <ios>
// #include "sha512.h"
#include <linux/fs.h>  // needed for BLKGETSIZE64
#include <sys/ioctl.h> // needed for ioctl()
#include <fcntl.h>     // needed for open()

/*
g++ -Wall -Wextra -std=gnu++11 ../lib/sha512.cpp test_read_device_file.cpp -o test_read_device_file
g++ -Wall -Wextra -std=gnu++11 ../lib/sha512.o   test_read_device_file.cpp -o test_read_device_file
*/

// std::string binary_digest_to_hex(unsigned char* digest) {
//     char buf[2*SHA512::DIGEST_SIZE+1];
//     buf[2*SHA512::DIGEST_SIZE] = 0;
//     for (int i = 0; i < SHA512::DIGEST_SIZE; i++)
//         sprintf(buf+i*2, "%02x", digest[i]);
//     return std::string(buf);
// }

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
	
	return 0;
}