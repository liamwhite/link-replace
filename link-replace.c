#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>

#define CHECK_ERROR(f,n) do { if ((f)) { perror(n); exit(1); } } while (0)

// https://stackoverflow.com/a/779960/3600108
// You must free the result if result is non-NULL.
char *str_replace(char *orig, char *rep, char *with)
{
    char *result; // the return string
    char *ins;    // the next insert point
    char *tmp;    // varies
    int len_rep;  // length of rep (the string to remove)
    int len_with; // length of with (the string to replace rep with)
    int len_front; // distance between rep and end of last rep
    int count;    // number of replacements

    // sanity checks and initialization
    if (!orig || !rep)
        return NULL;
    len_rep = strlen(rep);
    if (len_rep == 0)
        return NULL; // empty rep causes infinite loop during count
    if (!with)
        with = "";
    len_with = strlen(with);

    // count the number of replacements needed
    ins = orig;
    for (count = 0; tmp = strstr(ins, rep); ++count) {
        ins = tmp + len_rep;
    }

    tmp = result = malloc(strlen(orig) + (len_with - len_rep) * count + 1);
    CHECK_ERROR(result == NULL, "malloc");

    // first time through the loop, all the variable are set correctly
    // from here on,
    //    tmp points to the end of the result string
    //    ins points to the next occurrence of rep in orig
    //    orig points to the remainder of orig after "end of rep"
    while (count--) {
        ins = strstr(orig, rep);
        len_front = ins - orig;
        tmp = strncpy(tmp, orig, len_front) + len_front;
        tmp = strcpy(tmp, with) + len_with;
        orig += len_front + len_rep; // move to next "end of rep"
    }
    strcpy(tmp, orig);
    return result;
}

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
