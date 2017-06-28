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

## Example output

Watch me corrupt all these symlinks

```
liam@liam-Desktop:~/source/link-replace$ ./link-replace /tmp "/home/liam/Documents" "~/Documents"
/tmp/testdir/linkfinder.c: /home/liam/Documents/linkfinder.c -> ~/Documents/linkfinder.c
Error opening directory '/tmp/systemd-private-ac9ac8cae25348439cc1e79fda6eafa6-colord.service-a8AqMU' (Permission denied)
Error opening directory '/tmp/systemd-private-ac9ac8cae25348439cc1e79fda6eafa6-tor@default.service-Rky54R' (Permission denied)
/tmp/linkfinder.c: /home/liam/Documents/linkfinder.c -> ~/Documents/linkfinder.c
Error opening directory '/tmp/pulse-PKdhtXMmr18n' (Permission denied)
Error opening directory '/tmp/systemd-private-ac9ac8cae25348439cc1e79fda6eafa6-ntp.service-6KP3fm' (Permission denied)
Error opening directory '/tmp/systemd-private-ac9ac8cae25348439cc1e79fda6eafa6-rtkit-daemon.service-mQeRqs' (Permission denied)
```
