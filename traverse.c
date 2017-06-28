#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>

#include "error.h"
#include "replace.h"

void traverse(int dirfd, char *needle, char *replacewith)
{
    int newfd = 0;
    DIR *open_dir = fdopendir(dirfd);

    // Check for error opening it
    if (open_dir == NULL) {
        fprintf(stderr, "Error opening directory pointed to by handle %d (%s)\n", dirfd, strerror(errno));
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
                    fprintf(stderr, "Error opening directory '%s' (%s)\n", ent->d_name, strerror(errno));
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
