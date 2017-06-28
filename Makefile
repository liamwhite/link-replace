all: link-replace
link-replace: link-replace.o str-replace.o
link-replace.o: link-replace.c str-replace.h
str-replace.o: str-replace.c str-replace.h
