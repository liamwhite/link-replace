# link-replace

Find and replace a needle in symlink targets, recursively.

## Building

```
$ git clone https://github.com/liamwhite/link-replace.git
$ make
```

## Example usage

Decided to split off links onto a separately mounted disk and don't want several levels of indirection:

```
$ ./link-replace /mnt/files/ "/home/files/" "/mnt/files/"
```
