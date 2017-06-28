#ifndef REPLACE_H_INCLUDED
#define REPLACE_H_INCLUDED

// Update the symlink with name @name in open directory @dirfd to have
// a new target with @needle substituted with @replacewith.
//
// N.B.: This unlinks and recreates the file because symlinks cannot be edited.
void do_replace(int dirfd, char *name, char *needle, char *replacewith);

#endif
