#ifndef TRAVERSE_H_INCLUDED
#define TRAVERSE_H_INCLUDED

// Traverse a directory opened at @dirfd, scan it for symlinks, and
// if symlinks are found, call do_replace to replace them.
void traverse(int dirfd, char *needle, char *replacewith);

#endif
