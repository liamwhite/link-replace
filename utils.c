#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

char *fd_to_filename(int fd)
{
    struct stat buf;
    size_t targetsize = 0;
    char *targetname = NULL;
    char *name = NULL;

    asprintf(&name, "/proc/self/fd/%d", fd);
    lstat(name, &buf);

    targetsize = buf.st_size + 1;
    targetname = calloc(targetsize, 1);

    readlink(name, targetname, targetsize);

    free(name);
    return targetname;
}
