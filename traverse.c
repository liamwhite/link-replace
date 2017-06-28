#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "error.h"
#include "replace.h"
#include "utils.h"

void traverse(int dirfd, char *needle, char *replacewith)
{
    int newfd = 0;
    DIR *open_dir = fdopendir(dirfd);

    // Check for error opening it
    if (open_dir == NULL) {
        char *dirname = fd_to_filename(dirfd);
        fprintf(stderr, "Error reopening directory '%s' (%s)\n", dirname, strerror(errno));
        free(dirname);
        return;
    }

    for (struct dirent *ent; ent = readdir(open_dir); ent != NULL) {
        if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
            continue;

        switch (ent->d_type) {
            // Recurse
            case DT_DIR:
                newfd = openat(dirfd, ent->d_name, O_DIRECTORY);

                // If opening was successful, recurse into this directory
                if (newfd > 0) {
                    traverse(newfd, needle, replacewith);
                    close(newfd);
                } else {
                    char *dirname = fd_to_filename(dirfd);
                    fprintf(stderr, "Error opening directory '%s/%s' (%s)\n", dirname, ent->d_name, strerror(errno));
                    free(dirname);
                }

                break;

            // Operate
            case DT_LNK:
                do_replace(dirfd, ent->d_name, needle, replacewith);
                break;
        }
    }

    closedir(open_dir);
}
