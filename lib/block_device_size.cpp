//
// Created by j0sh on 11/22/15.
//

#include <linux/fs.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdio.h>
#include "block_device_size.h"

unsigned long long int get_special_file_size(const char* file_path) {
    FILE *fh = fopen(file_path, "r");
    unsigned long long int file_size1 = 0;
    unsigned long long int file_size2 = 0;
    fseek(fh, 0, SEEK_END);
    file_size2 = (unsigned long long int) ftell(fh);
    ioctl(fileno(fh), BLKGETSIZE64, &file_size1);
    return (file_size1 > file_size2) ? file_size1 : file_size2;
}