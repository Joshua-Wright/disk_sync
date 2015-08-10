#include <linux/fs.h>  // BLKGETSIZE64
#include <sys/ioctl.h> // ioctl()
#include <fcntl.h>     // open()
#include <string>      // string
#include "block_device_size.h"

/*
g++ -Wall -Wextra -std=gnu++11 -c     block_device_size.cpp -O3

g++ -Wall -Wextra -std=gnu++11 -c lib/block_device_size.cpp -o lib/block_device_size.o -O3
*/

unsigned long long int block_device_size(const std::string path) {
	int fh = open(path.c_str(), 0);
	unsigned long long int file_size_in_bytes = 0;
	ioctl(fh, BLKGETSIZE64, &file_size_in_bytes);
	return file_size_in_bytes;
}
