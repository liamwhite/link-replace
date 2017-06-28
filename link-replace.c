#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>

#include "str-replace.h"

#define CHECK_ERROR(f,n) do { if ((f)) { perror(n); exit(1); } } while (0)

void do_replace(int dirfd, char *name, char *needle, char *replacewith)
{
    struct stat statbuf;

    // Avoid race conditions on the symlink by holding a fd
    int fd = openat(dirfd, name, O_PATH | O_NOFOLLOW);
    if (fd == -1) return;

    // Find out how long the name of the target is
    int status = fstatat(fd, "", &statbuf, AT_EMPTY_PATH | AT_SYMLINK_NOFOLLOW);
    if (status == -1) return;

    // Allocate a buffer for the name of the old target
    size_t buflen = statbuf.st_size + 1;
    char *oldtarget = calloc(buflen, 1);
    CHECK_ERROR(oldtarget == NULL, "calloc");

    // Read the name of the target in.
    // N.B.: truncation is impossible here, and the buffer will be properly
    // nul-terminated.
    status = readlinkat(fd, "", oldtarget, buflen);
    if (status == -1) return;

    // String replace
    char *newtarget = str_replace(oldtarget, needle, replacewith);
    CHECK_ERROR(newtarget == NULL, "malloc");

    // Now recreate the symlink
    if (strcmp(oldtarget, newtarget) != 0) {
        CHECK_ERROR(unlinkat(dirfd, name, 0), "unlink");
        CHECK_ERROR(symlinkat(newtarget, dirfd, name), "symlink");
    }

    free(oldtarget);
    free(newtarget);
    close(fd);
}

void find_links(int dirfd, char *needle, char *replacewith)
{
    int newfd = 0;
    DIR *open_dir = fdopendir(dirfd);
    if (open_dir == NULL) return;

    for (struct dirent *ent; ent = readdir(open_dir); ent != NULL) {
        if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
            continue;

        switch (ent->d_type) {
            // Recurse
            case DT_DIR:
                newfd = openat(dirfd, ent->d_name, O_DIRECTORY);
                if (newfd > 0) find_links(newfd, needle, replacewith);
                close(newfd);
                break;

            // Operate
            case DT_LNK:
                do_replace(dirfd, ent->d_name, needle, replacewith);
                break;
        }
    }

    closedir(open_dir);
}

int main(int argc, char *argv[])
{
    if (argc != 3) {
        fprintf(stderr, "Usage: %s needle replacewith\n", argv[0]);
        return 2;
    }

    // Start in current working dir
    int cwdfd = open(".", O_DIRECTORY);
    if (cwdfd > 0) find_links(cwdfd, argv[1], argv[2]);
    close(cwdfd);
}
