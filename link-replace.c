#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>

#include "traverse.h"

int main(int argc, char *argv[])
{
    if (argc != 4) {
        fprintf(stderr, "Usage: %s dir needle replacewith\n", argv[0]);
        return 2;
    }

    // Start in dir
    int cwdfd = open(argv[1], O_DIRECTORY);

    if (cwdfd > 0) {
        traverse(cwdfd, argv[2], argv[3]);
        close(cwdfd);
    } else {
        perror("open");
        return 1;
    }
}
