//
// Created by j0sh on 11/22/15.
//

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <linux/fs.h>
#include "immutable.h"


void set_immutable(const char *filename) {
    FILE *fp = fopen(filename, "r");
    unsigned int flag = 0;
    ioctl(fileno(fp), FS_IOC_GETFLAGS, &flag);
    flag |= FS_IMMUTABLE_FL;
    ioctl(fileno(fp), FS_IOC_SETFLAGS, &flag);
    fclose(fp);
}

void set_mutable(const char *filename) {
    FILE *fp = fopen(filename, "r");
    unsigned int flag = 0;
    ioctl(fileno(fp), FS_IOC_GETFLAGS, &flag);
    flag &= ~FS_IMMUTABLE_FL;
    ioctl(fileno(fp), FS_IOC_SETFLAGS, &flag);
    fclose(fp);
}