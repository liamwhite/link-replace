#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "error.h"
#include "str-replace.h"

void do_replace(int dirfd, char *name, char *needle, char *replacewith)
{
    char *oldtarget = NULL;
    char *newtarget = NULL;
    size_t buflen = 0;
    struct stat statbuf;

    // Avoid race conditions on the symlink by holding a fd
    int fd = openat(dirfd, name, O_PATH | O_NOFOLLOW);

    if (fd < 0) {
        fprintf(stderr, "Failed to open '%s' (%s)", name, strerror(errno));
        goto end;
    }

    // Find out how long the name of the target is
    int status = fstatat(fd, "", &statbuf, AT_EMPTY_PATH | AT_SYMLINK_NOFOLLOW);

    if (status < 0) {
        fprintf(stderr, "Failed to stat() '%s' (%s)", name, strerror(errno));
        goto end;
    }

    // Allocate a buffer for the name of the old target
    buflen = statbuf.st_size + 1;
    oldtarget = calloc(buflen, 1);

    if (oldtarget == NULL) {
        perror("calloc");
        exit(1);
    }

    // Read the name of the target in.
    // N.B.: truncation is impossible here, and the buffer will be properly
    // nul-terminated.
    status = readlinkat(fd, "", oldtarget, buflen);

    if (status < 0) {
        fprintf(stderr, "Failed to readlink() '%s' (%s)", name, strerror(errno));
        goto end;
    }

    // String replace
    newtarget = str_replace(oldtarget, needle, replacewith);

    if (newtarget == NULL) {
        perror("malloc");
        exit(1);
    }

    // Now recreate the symlink
    if (strcmp(oldtarget, newtarget) != 0) {

        status = unlinkat(dirfd, name, 0);

        if (status < 0) {
            fprintf(stderr, "Failed to unlink() '%s' (%s)", name, strerror(errno));
            goto end;
        }

        status = symlinkat(newtarget, dirfd, name);

        if (status < 0) {
            fprintf(stderr, "Failed to symlink() '%s' (%s)", name, strerror(errno));
            goto end;
        }

        printf("%s: %s -> %s\n", name, oldtarget, newtarget);
    }

end:

    free(oldtarget);
    free(newtarget);
    close(fd);
}
