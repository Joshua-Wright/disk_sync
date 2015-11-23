//
// Created by j0sh on 11/22/15.
//
#include <iostream>
#include <linux/fs.h>
#include <sys/ioctl.h>
#include <fcntl.h>


int main(int argc, char** argv) {
    FILE *fh = fopen(argv[1], "r");
    unsigned long long int file_size_in_bytes = 0;
    ioctl(fileno(fh), BLKGETSIZE64, &file_size_in_bytes);

    fseek(fh, 0, SEEK_END);
    std::cout << ftell(fh) << std::endl;
    std::cout << file_size_in_bytes << std::endl;

}

